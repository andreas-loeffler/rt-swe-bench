#!/usr/bin/env python3
"""RT-SWE-Bench agent loop.

One run = one task x one condition x one sample. The agent works in its own
container on /ws/src/ros2_controllers with five actions: search, view, edit,
test (visible package tests only) and submit. The submitted `git diff` is
scored by scripts/evaluate.sh in a fresh container (hidden test, malloc-guard).

Conditions
  C0  issue + tools, zero-shot
  C1  C0 + a system prompt with real-time rules
  C2  C1 + after an allocating fix (tests pass, malloc-guard fires), the
      allocation report from the hidden probe (sites and symbolized backtrace);
      at most 3 repair rounds. The last submitted patch is scored.
  C3  C1 + a sixth action, calltree: a static call graph from the controller's
      update methods to every reachable heap allocation (harness/calltree).
      No runtime feedback, one round.

The model only needs an OpenAI-compatible /v1/chat/completions endpoint: actions
are a JSON object in the reply text, so no server-side tool parser is involved.

    scripts/agent.py --task tasks/rt-001 --condition C2 --sample 0 \
        --model google/gemma-4-31B-it-qat-w4a16-ct --base-url http://host:8000/v1
    scripts/agent.py --task tasks/rt-002 --condition C2 --replay harness/replay/rt-002.json

Results: results/runs/<model>/<condition>/<task>/sample_<k>.json (skipped if present).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import time
import urllib.error
import urllib.request
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
IMAGE = os.environ.get("IMAGE", "rt-swe-bench/base:jazzy-d71703b")
BACKEND = os.environ.get("RTSWE_BACKEND", "docker")  # docker | native (scripts/setup_native.sh)
WS = os.environ.get("RTSWE_WS", "/ws") if BACKEND == "native" else "/ws"
REPO = f"{WS}/src/ros2_controllers"
MAX_OBS = 6000          # characters per observation shown to the model
MAX_REPAIR_ROUNDS = 3   # C2
CALLTREE = ROOT / "harness/calltree/calltree.py"  # C3

SYSTEM_BASE = """You are a software engineer working on the ros2_controllers repository (ROS 2 Jazzy, C++).
The repository is checked out at {repo}. Fix the issue below by editing the source code.

You act through exactly one action per reply. Write a short line of reasoning, then the
action as a JSON object in a ```json fenced block. Available actions:

  {{"tool": "search", "args": {{"pattern": "<regex>", "path": "<dir or file, optional>"}}}}
      ripgrep over the repository, returns matching lines with file:line.
  {{"tool": "view", "args": {{"path": "<file>", "start": <line>, "end": <line>}}}}
      shows a file, at most 250 lines (start and end optional). Each line is printed as
      its line number, a tab, then the line exactly as it is in the file.
  {{"tool": "edit", "args": {{"path": "<file>", "old": "<exact text>", "new": "<replacement>"}}}}
      replaces one exact occurrence of old with new. old must match the file exactly,
      including indentation, and occur exactly once.
  {{"tool": "test", "args": {{"package": "<colcon package, optional>"}}}}
      builds the package and runs its tests (default: {package}). Returns the build
      errors or the test summary.
  {{"tool": "submit", "args": {{}}}}
      ends your work. Your current changes are the solution. Run the test action
      after your last edit first; submit is refused otherwise.

Paths are relative to the repository root. Keep changes minimal and keep the existing
behavior and public API unless the issue says otherwise. Before you submit, run the test
action and check that the package builds and its tests pass."""

SYSTEM_RT = """

This code runs in a hard real-time control loop. The controller's update() method (and
everything it calls) runs once per control cycle, often at 1 kHz, and must not allocate
or free heap memory: no new/delete or malloc, no std::vector or std::string growth, no
copies of containers or ROS messages that contain vectors or strings, no std::function
that cannot store its callable inline, no exceptions, and no logging that formats strings.
Allocate and size buffers in on_configure() or on_activate() instead, and reuse them in
update(). Allocation outside update() is fine."""

SYSTEM_CALLTREE = """

You have one more action:

  {{"tool": "calltree", "args": {{"package": "<colcon package, optional>", "function": "<name, optional>"}}}}
      static analysis of the current code: compiles the package with clang and walks the
      call graph from the controller's update methods. Lists every call site in the package
      from which a heap allocation can be reached, with the call chain down to the
      allocation, including allocations hidden inside library calls, message copies and
      std::string. Pass "function" to see only one function and what it calls. It
      over-approximates: some listed paths never run (error branches, buffers that only
      grow once)."""

FEEDBACK_C2 = """Your patch was checked with malloc-guard around every update() call of the active
controller. update() still allocates heap memory:

{report}

Fix this without breaking the functional behavior, then submit again. You have the same
actions as before. Repair round {round} of {max_rounds}."""

# Ablations of C2 (Track 2): C2B says only that update() allocates, without location;
# C2N gives the same number of extra rounds with a generic review request, after every
# submission, so it leaks neither the functional nor the real-time verdict.
FEEDBACK_C2B = """Your patch was checked with malloc-guard around every update() call of the active
controller. update() still allocates heap memory: {bytes} bytes in {cycles} of {total} cycles.

Fix this without breaking the functional behavior, then submit again. You have the same
actions as before. Repair round {round} of {max_rounds}."""

FEEDBACK_C2N = """Before the patch is accepted, review it once more: check that it fully resolves the
issue and introduces no new problems, then submit again. You have the same actions as
before. Review round {round} of {max_rounds}."""

# C4, oracle localization: the malloc-guard report of the unpatched code (tasks/<id>/oracle.txt)
# is appended to the issue before the first action.
ORACLE_C4 = """

malloc-guard, run around every update() call on the unpatched code, reports:

{report}"""

REPAIR_CONDS = ("C2", "C2B", "C2N")


# --------------------------------------------------------------------------- container

class Box:
    """A long-lived container the agent works in."""

    def __init__(self, image: str):
        self.name = f"rtswe-agent-{uuid.uuid4().hex[:10]}"
        subprocess.run(["docker", "run", "-d", "--rm", "--name", self.name, "--entrypoint", "sleep",
                        image, "infinity"], check=True, capture_output=True)
        self.calltree = None  # path of calltree.py inside the container, copied on first use

    def calltree_script(self) -> str:
        if self.calltree is None:
            self.write("/tmp/rtswe_calltree.py", CALLTREE.read_text())
            self.calltree = "/tmp/rtswe_calltree.py"
        return self.calltree

    def sh(self, cmd: str, timeout: int = 900) -> tuple[int, str]:
        full = f"source /opt/ros/jazzy/setup.bash && source /ws/install/setup.bash && {cmd}"
        try:
            p = subprocess.run(["docker", "exec", self.name, "bash", "-lc", full],
                               capture_output=True, text=True, timeout=timeout)
            return p.returncode, p.stdout + p.stderr
        except subprocess.TimeoutExpired:
            return 124, f"(timed out after {timeout} s)"

    def read(self, path: str) -> str | None:
        p = subprocess.run(["docker", "exec", self.name, "cat", path], capture_output=True, text=True)
        return p.stdout if p.returncode == 0 else None

    def write(self, path: str, content: str) -> None:
        subprocess.run(["docker", "exec", "-i", self.name, "bash", "-c", f"cat > {shlex.quote(path)}"],
                       input=content, text=True, check=True)

    def close(self) -> None:
        subprocess.run(["docker", "rm", "-f", self.name], capture_output=True)


class LocalBox:
    """Native backend: a host workspace ($RTSWE_WS, default /ws), reset to the
    pinned state first. One run per workspace at a time. The agent's tools are the same and stay inside the
    repository; the hidden tests are copied into /ws/src only while scoring.
    """

    def __init__(self):
        subprocess.run(["rm", "-rf", f"{WS}/src/rtswe_eval"], check=True)
        subprocess.run(["bash", str(ROOT / "scripts/native_reset.sh")], check=True, capture_output=True)

    def calltree_script(self) -> str:
        return str(CALLTREE)

    def sh(self, cmd: str, timeout: int = 900) -> tuple[int, str]:
        try:
            p = subprocess.run(["bash", "-c", f"source /opt/rtswe/env.sh && {cmd}"],
                               capture_output=True, text=True, timeout=timeout)
            return p.returncode, p.stdout + p.stderr
        except subprocess.TimeoutExpired:
            return 124, f"(timed out after {timeout} s)"

    def read(self, path: str) -> str | None:
        try:
            return Path(path).read_text()
        except OSError:
            return None

    def write(self, path: str, content: str) -> None:
        Path(path).write_text(content)

    def close(self) -> None:
        subprocess.run(["rm", "-rf", f"{WS}/src/rtswe_eval"])


def apply_task_base(box, task_dir: Path, task: dict) -> None:
    """Track 1: put the task's starting point (base patch) in place and commit it,
    so the agent's `git diff` holds only its own changes."""
    base = task.get("base_patch")
    if not base:
        return
    # one file per process: native workers share /tmp, and parallel runs that start in the
    # same second would otherwise overwrite or delete each other's base patch
    diff = f"/tmp/rtswe_base_{os.getpid()}.diff"
    box.write(diff, (task_dir / base).read_text())
    code, out = box.sh(
        f"cd {REPO} && git apply --whitespace=nowarn {diff} && git add -A && "
        "git -c user.name=rtswe -c user.email=rtswe@localhost commit -qm 'rtswe: task base' && "
        f"rm {diff}", timeout=120)
    if code != 0:
        raise RuntimeError(f"task base did not apply: {out[-500:]}")


def clip(text: str, limit: int = MAX_OBS) -> str:
    if len(text) <= limit:
        return text
    half = limit // 2
    return text[:half] + f"\n... ({len(text) - limit} characters omitted) ...\n" + text[-half:]


def repo_path(rel: str) -> str | None:
    rel = (rel or "").strip().lstrip("/")
    if rel.startswith("ws/src/ros2_controllers/"):
        rel = rel[len("ws/src/ros2_controllers/"):]
    if ".." in Path(rel).parts:
        return None
    return f"{REPO}/{rel}" if rel else REPO


# --------------------------------------------------------------------------- tools

def tool_search(box: Box, args: dict) -> str:
    pattern = str(args.get("pattern", ""))
    if not pattern:
        return "error: pattern is required"
    path = repo_path(str(args.get("path", "")))
    if path is None:
        return "error: path must stay inside the repository"
    code, out = box.sh(f"cd {REPO} && rg -n --no-heading -g '!*.md' -g '!*.rst' -m 20 "
                       f"-e {shlex.quote(pattern)} {shlex.quote(os.path.relpath(path, REPO))} | head -80",
                       timeout=60)
    return clip(out) if out.strip() else "no matches"


def tool_view(box: Box, args: dict) -> str:
    path = repo_path(str(args.get("path", "")))
    if path is None or path == REPO:
        return "error: give a file path inside the repository"
    text = box.read(path)
    if text is None:
        return f"error: cannot read {args.get('path')}"
    lines = text.splitlines()
    start = max(1, int(args.get("start") or 1))
    end = min(len(lines), int(args.get("end") or start + 249), start + 249)
    # cat -n format: the line number and a tab, then the line exactly as in the file.
    body = "\n".join(f"{i:6d}\t{lines[i - 1]}" for i in range(start, end + 1))
    return f"{args.get('path')} (lines {start}-{end} of {len(lines)})\n{body}"


def edit_hint(text: str, old: str) -> str:
    """Where the first line of `old` occurs in the file, ignoring indentation."""
    first = next((l for l in old.splitlines() if l.strip()), "").strip()
    if not first:
        return ""
    lines = text.splitlines()
    hits = [(i, l) for i, l in enumerate(lines, 1) if l.strip() == first]
    hits = hits or [(i, l) for i, l in enumerate(lines, 1) if first in l]
    if not hits:
        return ""
    shown = "\n".join(f"  line {i}: {json.dumps(l)}" for i, l in hits[:5])
    return ("\nLines in the file that match the first line of old, with their exact indentation "
            "(JSON-quoted):\n" + shown)


def tool_edit(box: Box, args: dict) -> str:
    path = repo_path(str(args.get("path", "")))
    old, new = args.get("old"), args.get("new")
    if path is None or path == REPO or not isinstance(old, str) or not isinstance(new, str):
        return "error: edit needs path, old and new"
    text = box.read(path)
    if text is None:
        return f"error: cannot read {args.get('path')}"
    n = text.count(old)
    if n == 0:
        return (f"error: old text occurs 0 times in {args.get('path')}; it must match the file "
                f"exactly, including indentation." + edit_hint(text, old))
    if n > 1:
        at, pos = [], text.find(old)
        while pos != -1:
            at.append(text.count("\n", 0, pos) + 1)
            pos = text.find(old, pos + 1)
        return (f"error: old text occurs {n} times in {args.get('path')} (starting at lines "
                f"{', '.join(map(str, at))}); include more surrounding lines so it is unique")
    box.write(path, text.replace(old, new, 1))
    return f"edited {args.get('path')}"


def tool_test(box: Box, args: dict, default_pkg: str) -> str:
    pkg = str(args.get("package") or default_pkg)
    if not re.fullmatch(r"[a-z0-9_]+", pkg):
        return "error: invalid package name"
    code, out = box.sh(
        f"cd {WS} && echo {pkg} >> {WS}/.rtswe_dirty && colcon build --packages-select {pkg} --cmake-args -G Ninja "
        f"-DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON 2>&1 | tail -n 60", timeout=1200)
    if "Failed" in out or "error:" in out:
        # Ninja prints compiler output on stdout, which colcon keeps out of the console;
        # show the error lines with context from the package's build log instead.
        _, log = box.sh(
            f"grep -B2 -A6 -E ': (fatal )?error:|undefined reference|CMake Error|ninja: build stopped' "
            f"{WS}/log/latest_build/{pkg}/stdout_stderr.log | grep -v -E '^(ccache |/usr/bin/c\\+\\+ |FAILED: )' "
            f"| head -n 150", timeout=60)
        return "build failed. Compiler errors:\n" + clip(log.strip() or out)
    code, out = box.sh(
        f"cd {WS} && colcon test --packages-select {pkg} > /dev/null 2>&1; "
        f"colcon test-result --test-result-base build/{pkg} --verbose 2>&1 | tail -n 80", timeout=1200)
    return clip(out) if out.strip() else "tests ran, no results reported"


def tool_calltree(box: Box, args: dict, default_pkg: str) -> str:
    pkg = str(args.get("package") or default_pkg)
    if not re.fullmatch(r"[a-z0-9_]+", pkg):
        return "error: invalid package name"
    fn = str(args.get("function") or "").strip()
    if fn and not re.fullmatch(r"[A-Za-z0-9_:~]{1,100}", fn):
        return "error: function must be a C++ name such as update_and_write_commands or Odometry::update"
    cmd = f"python3 {box.calltree_script()} --package {pkg} --ws {WS}"
    if fn:
        cmd += f" --function {shlex.quote(fn)}"
    _, out = box.sh(cmd, timeout=600)
    return clip(out.strip() or "calltree produced no output")


# --------------------------------------------------------------------------- model

class ContextFull(Exception):
    """The conversation no longer fits the server's context window."""


class Endpoint:
    def __init__(self, base_url: str, model: str, api_key: str, temperature: float, top_p: float,
                 top_k: int, max_tokens: int):
        self.url = base_url.rstrip("/") + "/chat/completions"
        self.model, self.api_key = model, api_key
        self.temperature, self.top_p, self.top_k, self.max_tokens = temperature, top_p, top_k, max_tokens

    def __call__(self, messages: list[dict], seed: int) -> tuple[str, dict]:
        body = {"model": self.model, "messages": messages, "temperature": self.temperature,
                "top_p": self.top_p, "top_k": self.top_k, "max_tokens": self.max_tokens, "seed": seed}
        if THINKING:  # Gemma 4: the chat template puts <|think|> into the system turn
            body["chat_template_kwargs"] = {"enable_thinking": True}
        # Other model families: their model card's sampling settings and template
        # switches, e.g. RTSWE_SAMPLING='{"temperature": 0.7, "top_p": 0.8, "top_k": 20, "presence_penalty": 1.5}'
        # and RTSWE_CHAT_KWARGS='{"enable_thinking": false}' (Qwen3.x think by default).
        body.update(SAMPLING)
        if CHAT_KWARGS:
            body["chat_template_kwargs"] = {**body.get("chat_template_kwargs", {}), **CHAT_KWARGS}
        req = urllib.request.Request(self.url, data=json.dumps(body).encode(), method="POST",
                                     headers={"Content-Type": "application/json",
                                              "Authorization": f"Bearer {self.api_key}"})
        for attempt in range(6):
            try:
                with urllib.request.urlopen(req, timeout=600) as r:
                    data = json.loads(r.read())
                msg = data["choices"][0]["message"]
                self.last_reasoning = msg.get("reasoning") or msg.get("reasoning_content") or ""
                return msg.get("content") or "", data.get("usage", {})
            except urllib.error.HTTPError as e:
                detail = e.read().decode(errors="replace")
                if e.code == 400 and ("context" in detail.lower() or "max_model_len" in detail
                                      or "maximum" in detail.lower()):
                    raise ContextFull(detail[:300]) from e
                if attempt == 5:
                    raise
                time.sleep(10 * (attempt + 1))
                print(f"  endpoint HTTP {e.code}: {detail[:200]}, retrying", file=sys.stderr)
            except (urllib.error.URLError, TimeoutError, ConnectionError) as e:
                if attempt == 5:
                    raise
                time.sleep(10 * (attempt + 1))
                print(f"  endpoint error ({e}), retrying", file=sys.stderr)
        raise RuntimeError("unreachable")


class Replay:
    """Scripted policy for testing the harness without a model."""

    def __init__(self, path: Path):
        self.steps = json.loads(path.read_text())
        self.i = 0

    def __call__(self, messages: list[dict], seed: int) -> tuple[str, dict]:
        step = self.steps[min(self.i, len(self.steps) - 1)]
        self.i += 1
        return "replay\n```json\n" + json.dumps(step) + "\n```", {}


THOUGHT = re.compile(r"<\|channel>thought.*?<channel\|>", flags=re.S)
# Thinking on: RTSWE_THINKING=1. Needs a server started with
# --reasoning-parser gemma4, which returns the thought in message.reasoning. Results go to a separate
# model directory (<model>-thinking); earlier turns keep only the final answer, as the model card asks.
THINKING = os.environ.get("RTSWE_THINKING") == "1"
SAMPLING = json.loads(os.environ.get("RTSWE_SAMPLING") or "{}")
CHAT_KWARGS = json.loads(os.environ.get("RTSWE_CHAT_KWARGS") or "{}")
THINK_TAG = re.compile(r"<think>.*?</think>", flags=re.S)  # Qwen, should a thought leak into content


def parse_action(reply: str) -> dict | None:
    blocks = re.findall(r"```(?:json)?\s*(\{.*?\})\s*```", reply, flags=re.S)
    candidates = blocks[::-1] or re.findall(r"(\{\s*\"tool\".*\})", reply, flags=re.S)
    for c in candidates:
        try:
            a = json.loads(c)
        except json.JSONDecodeError:
            continue
        if isinstance(a, dict) and isinstance(a.get("tool"), str):
            a.setdefault("args", {})
            return a
    return None


# --------------------------------------------------------------------------- scoring

def evaluate(task_dir: Path, patch: str, log_dir: Path) -> dict:
    log_dir.mkdir(parents=True, exist_ok=True)
    patch_file = log_dir / "patch.diff"
    patch_file.write_text(patch)
    p = subprocess.run([str(ROOT / "scripts/evaluate.sh"), str(task_dir),
                        str(patch_file) if patch.strip() else "none"],
                       capture_output=True, text=True, env={**os.environ, "LOG_DIR": str(log_dir)})
    if p.returncode != 0:
        return {"F": 0, "R": 0, "error": p.stderr[-2000:]}
    return json.loads(p.stdout.strip().splitlines()[-1])


def allocation_report(result: dict, log_dir: Path) -> str:
    """Sites and the first symbolized backtrace from the hidden probe's log."""
    alloc = result.get("alloc") or {}
    lines = [f"{alloc.get('allocating_cycles', '?')} of {alloc.get('cycles', '?')} update() cycles allocated, "
             f"{alloc.get('allocs', '?')} allocations, {alloc.get('bytes', '?')} bytes in total."]
    for s in sorted(alloc.get("sites", []), key=lambda s: -s["allocs"])[:8]:
        lines.append(f"  {s['allocs']} allocations ({s['bytes']} bytes) at {s['site']}")
    log = log_dir / "probe.log"
    if log.exists():
        trace, grab = [], False
        for raw in log.read_text(errors="replace").splitlines():
            line = re.sub(r"^\d+: ", "", raw)
            if line.startswith("[rtswe] site:"):
                if grab:
                    break
                grab = True
                continue
            if grab and line.startswith("  #"):
                # keep the frames that say something: skip the allocator hook, the
                # hidden test itself, gtest and libc startup frames
                if any(k in line for k in ("rtswe_eval", "malloc_guard", "HandleMallocGuardViolation",
                                           "TestBody", "testing::", "__libc", "main at", "_start")):
                    continue
                trace.append(re.sub(r" \[[^\]]+\+0x[0-9a-f]+\]$", "", line))
        if trace:
            lines.append("Backtrace of the first allocation site:")
            lines.extend(trace[:14])
    return "\n".join(lines)


# --------------------------------------------------------------------------- run

def run(args: argparse.Namespace) -> dict:
    task_dir = Path(args.task).resolve()
    task = json.loads((task_dir / "task.json").read_text())
    issue = (task_dir / task["issue"]).read_text()
    model_tag = "replay" if args.replay else args.model.split("/")[-1] + ("-thinking" if THINKING else "")
    out = ROOT / "results/runs" / model_tag / args.condition / task["id"] / f"sample_{args.sample}.json"
    if out.exists() and not args.force:
        print(f"skip {out} (exists)")
        return json.loads(out.read_text())
    out.parent.mkdir(parents=True, exist_ok=True)

    policy = Replay(Path(args.replay)) if args.replay else Endpoint(
        args.base_url, args.model, args.api_key, args.temperature, args.top_p, args.top_k,
        args.max_tokens)
    system = SYSTEM_BASE.format(repo=REPO, package=task["package"])
    if args.condition != "C0":
        system += SYSTEM_RT
    if args.condition == "C3":
        system += SYSTEM_CALLTREE.format()
    if args.condition == "C4":
        issue += ORACLE_C4.format(report=(task_dir / "oracle.txt").read_text().rstrip())
    messages = [{"role": "system", "content": system},
                {"role": "user", "content": f"Issue in package `{task['package']}`:\n\n{issue}"}]

    record = {"task": task["id"], "condition": args.condition, "sample": args.sample,
              "sampling": {"temperature": args.temperature, "top_p": args.top_p, "top_k": args.top_k,
                           "max_tokens": args.max_tokens, **SAMPLING, "chat_template_kwargs": CHAT_KWARGS},
              "model": args.model if not args.replay else "replay", "rounds": [], "steps": 0,
              "usage": {"prompt_tokens": 0, "completion_tokens": 0}, "started": time.time()}
    box = Box(IMAGE) if BACKEND == "docker" else LocalBox()
    try:
        apply_task_base(box, task_dir, task)
        max_rounds = 1 + (MAX_REPAIR_ROUNDS if args.condition in REPAIR_CONDS else 0)
        edited_since_test = False  # submit is refused until the current edits were tested
        for rnd in range(max_rounds):
            submitted = False
            for _ in range(args.max_steps):
                try:
                    reply, usage = policy(messages, seed=args.sample * 1000 + record["steps"])
                except ContextFull as e:
                    record["context_full"] = str(e)
                    break
                record["steps"] += 1
                for k in record["usage"]:
                    record["usage"][k] += usage.get(k, 0)
                # Gemma 4: earlier turns keep only the final answer, not the thought block.
                reply = THINK_TAG.sub("", THOUGHT.sub("", reply)).strip()
                if THINKING:
                    record.setdefault("thoughts", []).append(getattr(policy, "last_reasoning", ""))
                messages.append({"role": "assistant", "content": reply})
                action = parse_action(reply)
                if action is None:
                    obs = "error: no action found. Reply with one JSON action in a ```json block."
                else:
                    tool, targs = action["tool"], action["args"] if isinstance(action["args"], dict) else {}
                    if tool == "submit" and edited_since_test:
                        obs = ("error: you changed files since your last test run. Run the test "
                               "action on your current changes before you submit.")
                    elif tool == "submit":
                        submitted = True
                        break
                    else:
                        tools = {"search": lambda: tool_search(box, targs), "view": lambda: tool_view(box, targs),
                                 "edit": lambda: tool_edit(box, targs),
                                 "test": lambda: tool_test(box, targs, task["package"])}
                        if args.condition == "C3":
                            tools["calltree"] = lambda: tool_calltree(box, targs, task["package"])
                        obs = tools.get(tool, lambda: f"error: unknown tool {tool!r}")()
                        if tool == "edit" and obs.startswith("edited"):
                            edited_since_test = True
                        elif tool == "test":
                            edited_since_test = False
                messages.append({"role": "user", "content": obs})
            _, patch = box.sh(f"cd {REPO} && git diff", timeout=60)
            log_dir = out.parent / f"sample_{args.sample}_round{rnd}"
            result = evaluate(task_dir, patch, log_dir)
            record["rounds"].append({"round": rnd, "submitted": submitted, "steps": record["steps"],
                                     "F": result.get("F", 0), "R": result.get("R", 0),
                                     "result": result, "patch": patch})
            print(f"  round {rnd}: submitted={submitted} F={result.get('F')} R={result.get('R')} "
                  f"steps={record['steps']}")
            # C2 repairs allocating fixes only: the patch builds and passes the tests (F=1)
            # but malloc-guard saw an allocation (R=0). Anything else is final.
            allocating_fix = result.get("F") == 1 and result.get("R") == 0
            # C2N repairs after every submission, without saying anything about the verdict.
            again = submitted if args.condition == "C2N" else allocating_fix
            if args.condition not in REPAIR_CONDS or not again or rnd == max_rounds - 1:
                break
            if args.condition == "C2":
                fb = FEEDBACK_C2.format(report=allocation_report(result, log_dir),
                                        round=rnd + 1, max_rounds=MAX_REPAIR_ROUNDS)
            elif args.condition == "C2B":
                a = result.get("alloc") or {}
                fb = FEEDBACK_C2B.format(bytes=a.get("bytes", "?"), cycles=a.get("allocating_cycles", "?"),
                                         total=a.get("cycles", "?"), round=rnd + 1, max_rounds=MAX_REPAIR_ROUNDS)
            else:
                fb = FEEDBACK_C2N.format(round=rnd + 1, max_rounds=MAX_REPAIR_ROUNDS)
            messages.append({"role": "user", "content": fb})
    finally:
        box.close()

    last = record["rounds"][-1]
    record.update({"F": last["F"], "R": last["R"], "repair_rounds": len(record["rounds"]) - 1,
                   "seconds": round(time.time() - record["started"], 1), "messages": messages})
    out.write_text(json.dumps(record, indent=1))
    print(f"{task['id']} {args.condition} sample {args.sample}: F={record['F']} R={record['R']} -> {out}")
    return record


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--task", required=True)
    ap.add_argument("--condition", choices=["C0", "C1", "C2", "C3", "C4", "C2B", "C2N"], required=True)
    ap.add_argument("--sample", type=int, default=0)
    ap.add_argument("--model", default="google/gemma-4-31B-it-qat-w4a16-ct")
    ap.add_argument("--base-url", default=os.environ.get("RTSWE_BASE_URL", "http://localhost:8000/v1"))
    ap.add_argument("--api-key", default=os.environ.get("RTSWE_API_KEY", "none"))
    ap.add_argument("--temperature", type=float, default=1.0)
    ap.add_argument("--top-p", type=float, default=0.95)
    ap.add_argument("--top-k", type=int, default=64)
    ap.add_argument("--max-tokens", type=int, default=8192 if THINKING else 4096,
                    help="per reply; thinking on: 8192, so the thought fits")
    ap.add_argument("--max-steps", type=int, default=40, help="actions per round")
    ap.add_argument("--replay", help="JSON list of actions instead of a model (harness test)")
    ap.add_argument("--force", action="store_true", help="rerun even if the result file exists")
    run(ap.parse_args())


if __name__ == "__main__":
    main()
