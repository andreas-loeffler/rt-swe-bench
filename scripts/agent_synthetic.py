#!/usr/bin/env python3
"""Agent runs on the generated synthetic tasks (accepted candidates of synthetic/generator/generate.py).

Tasks, built from every accepted candidate directory:
  implement  kernel.hpp starts as the stub; implement the specification (spec.md says the
             real-time methods must not allocate)
  remediate  kernel.hpp starts as one kept allocating variant (it passes the functional test);
             make the real-time methods allocation-free without changing behavior
The test (test.cpp) stays hidden. The agent has four actions: view (spec.md, kernel.hpp),
edit (kernel.hpp), test (compiles kernel.hpp into the hidden test and reports the build errors
or the functional result only, like the visible tests of the repository tier) and submit.
The submitted kernel.hpp is scored with synthetic/score.sh: F (functional check) and R (no
allocation in any guarded real-time call).

Conditions (as in the repository tier, without C3)
  C0  task + tools
  C1  C0 + the real-time system prompt
  C2  C1 + after a submission with F=1 and R=0, the allocation report (calls, sites); at
      most 3 repair rounds. The last submission is scored.

    scripts/agent_synthetic.py run --cands OUT/candidates --model google/gemma-4-31B-it-qat-w4a16-ct \\
        --base-url http://localhost:8000/v1 --conditions C0,C1,C2 --samples 1 --workers 12 [--shard k/n]
    scripts/agent_synthetic.py run --offline --cands OUT/candidates --model ... # model in-process, no server
    scripts/agent_synthetic.py report --runs results/synthetic_runs

Scoring backend: GEN_BACKEND=native (malloc-guard in $MALLOC_GUARD) or docker (default).
Results: results/synthetic_runs/<model>/<condition>/<task>/sample_<k>.json (skipped if present).
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import hashlib
import json
import os
import random
import shutil
import sys
import tempfile
import threading
import time
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
sys.path.insert(0, str(ROOT / "synthetic" / "generator"))
from agent import THOUGHT, ContextFull, Endpoint, clip, edit_hint, parse_action  # noqa: E402
from generate import score  # noqa: E402

MAX_REPAIR_ROUNDS = 3
LOCK = threading.Lock()

SYSTEM_BASE = """You are a C++ engineer. You work on one header-only C++20 file, kernel.hpp, that
implements the class described in spec.md. A hidden test compiles kernel.hpp and checks its
results. Complete the task below by editing kernel.hpp.

You act through exactly one action per reply. Write a short line of reasoning, then the
action as a JSON object in a ```json fenced block. Available actions:

  {"tool": "view", "args": {"path": "<spec.md or kernel.hpp>", "start": <line>, "end": <line>}}
      shows a file, at most 250 lines (start and end optional). Each line is printed as
      its line number, a tab, then the line exactly as it is in the file.
  {"tool": "edit", "args": {"path": "kernel.hpp", "old": "<exact text>", "new": "<replacement>"}}
      replaces one exact occurrence of old with new. old must match the file exactly,
      including indentation, and occur exactly once.
  {"tool": "test", "args": {}}
      compiles kernel.hpp into the hidden test and runs it. Returns the compiler errors or
      whether the functional checks pass (and the first failing check).
  {"tool": "submit", "args": {}}
      ends your work. The current kernel.hpp is the solution. Run the test action after
      your last edit first; submit is refused otherwise.

Keep the public API (class name, method signatures) exactly as in spec.md."""

SYSTEM_RT = """

This code runs on a hard real-time path. The real-time methods (everything except
configure(...)) run once per cycle, often at 1 kHz or at audio rate, and must not allocate
or free heap memory: no new/delete or malloc, no std::vector or std::string growth, no copies
of containers or strings, no std::function that cannot store its callable inline, no
exceptions. Allocate and size buffers in configure(...) instead, and reuse them in the
real-time methods. Allocation in configure(...) is fine."""

TASK_IMPLEMENT = """Implement the class in kernel.hpp so that it meets the specification in spec.md.
kernel.hpp currently holds a stub with the public API and TODOs."""

TASK_REMEDIATE = """kernel.hpp implements the specification in spec.md and passes the functional test,
but its real-time methods allocate heap memory, which the specification forbids. Make them
allocation-free without changing the results or the public API."""

FEEDBACK_C2 = """Your kernel.hpp was checked with malloc-guard around every real-time call of the
hidden test. The real-time methods still allocate heap memory:

{report}

Fix this without breaking the functional behavior, then submit again. You have the same
actions as before. Repair round {round} of {max_rounds}."""


def tasks_from(cands: Path) -> list[dict]:
    out = []
    for p in sorted(cands.glob("*/provenance.json")):
        prov = json.loads(p.read_text())
        if not prov.get("accepted"):
            continue
        kdir = p.parent
        base = {"kdir": str(kdir), "kernel": prov["id"], "generator": prov["generator"],
                "level": prov["idea"].get("level", "?"), "domain": prov["idea"].get("domain", "?")}
        out.append({**base, "id": f"{prov['id']}__implement", "kind": "implement",
                    "start": str(kdir / "stub.hpp")})
        for v in prov["gates"]["G3"].get("kept", []):
            out.append({**base, "id": f"{prov['id']}__remediate_{Path(v).stem}", "kind": "remediate",
                        "start": str(kdir / "variants" / v)})
    return out


def view(files: dict, args: dict) -> str:
    name = str(args.get("path", "")).split("/")[-1]
    if name not in files:
        return "error: path must be spec.md or kernel.hpp"
    lines = files[name].splitlines()
    try:
        start = max(1, int(args.get("start") or 1))
        end = min(len(lines), int(args.get("end") or start + 249), start + 249)
    except (TypeError, ValueError):
        return "error: start and end must be line numbers"
    body = "\n".join(f"{i:6d}\t{lines[i - 1]}" for i in range(start, end + 1))
    return f"{name} (lines {start}-{end} of {len(lines)})\n{body}"


def edit(files: dict, args: dict) -> str:
    name = str(args.get("path", "")).split("/")[-1]
    old, new = args.get("old"), args.get("new")
    if name != "kernel.hpp":
        return "error: only kernel.hpp can be edited"
    if not isinstance(old, str) or not isinstance(new, str):
        return "error: edit needs path, old and new"
    text = files[name]
    n = text.count(old)
    if n == 0:
        return ("error: old text occurs 0 times in kernel.hpp; it must match the file exactly, "
                "including indentation." + edit_hint(text, old))
    if n > 1:
        return f"error: old text occurs {n} times in kernel.hpp; include more surrounding lines so it is unique"
    files[name] = text.replace(old, new, 1)
    return "edited kernel.hpp"


def check(run_dir: Path, kernel: str) -> dict:
    """Compile kernel into the hidden test and run it; the full score.sh result."""
    (run_dir / "kernel.hpp").write_text(kernel)
    return score(run_dir, [run_dir / "kernel.hpp"]).get("kernel.hpp") or {"build": 0, "F": 0, "R": 0}


def test_obs(res: dict) -> str:
    if not res.get("build"):
        return "build failed. Compiler errors:\n" + clip(res.get("build_error") or "(no message)")
    if res.get("F"):
        return "build ok. All functional checks pass."
    mm = res.get("mismatch")
    return ("build ok. A functional check failed: " + mm) if mm else \
        "build ok. The test did not finish (crash or timeout) or reported no result."


def alloc_report(res: dict) -> str:
    a = res.get("alloc") or {}
    lines = [f"{a.get('allocating_cycles', '?')} of {a.get('cycles', '?')} real-time calls allocated, "
             f"{a.get('allocs', '?')} allocations, {a.get('bytes', '?')} bytes in total."]
    sites = [s for s in sorted(a.get("sites", []), key=lambda s: -s.get("allocs", 0))
             if "kernel.hpp" in str(s.get("site"))]  # never point into the hidden test
    for s in sites[:8]:
        lines.append(f"  {s.get('allocs')} allocations at {str(s.get('site')).replace('/tmp/', '')}")
    if not sites:
        lines.append("The allocating code is inlined into the test's real-time calls; look at the "
                     "real-time methods of kernel.hpp and everything they call.")
    return "\n".join(lines)


def run_steps(task: dict, cond: str, sample: int, out: Path, max_steps: int):
    """One agent run as a generator: yields (messages, seed) whenever it needs a model reply and receives
    (reply, usage), or None when the conversation no longer fits the context. Returns the record."""
    kdir = Path(task["kdir"])
    files = {"spec.md": (kdir / "spec.md").read_text(), "kernel.hpp": Path(task["start"]).read_text()}
    system = SYSTEM_BASE + (SYSTEM_RT if cond in ("C1", "C2") else "")
    messages = [{"role": "system", "content": system},
                {"role": "user", "content": TASK_IMPLEMENT if task["kind"] == "implement" else TASK_REMEDIATE}]
    rec = {**{k: task[k] for k in ("id", "kernel", "kind", "level", "domain", "generator")},
           "condition": cond, "sample": sample, "rounds": [], "steps": 0,
           "usage": {"prompt_tokens": 0, "completion_tokens": 0}, "started": time.time()}
    seed0 = int(hashlib.sha256(f"{task['id']}/{cond}/{sample}".encode()).hexdigest()[:6], 16)
    with tempfile.TemporaryDirectory() as tmp:
        run_dir = Path(tmp)
        shutil.copy(kdir / "test.cpp", run_dir / "test.cpp")
        max_rounds = 1 + (MAX_REPAIR_ROUNDS if cond == "C2" else 0)
        edited_since_test = False
        for rnd in range(max_rounds):
            submitted = False
            for _ in range(max_steps):
                got = yield messages, seed0 + rec["steps"]
                if got is None:
                    rec["context_full"] = True
                    break
                reply, usage = got
                rec["steps"] += 1
                for k in rec["usage"]:
                    rec["usage"][k] += usage.get(k, 0)
                reply = THOUGHT.sub("", reply).strip()
                messages.append({"role": "assistant", "content": reply})
                action = parse_action(reply)
                if action is None:
                    obs = "error: no action found. Reply with one JSON action in a ```json block."
                else:
                    tool = action["tool"]
                    targs = action["args"] if isinstance(action["args"], dict) else {}
                    if tool == "submit" and edited_since_test:
                        obs = ("error: you changed kernel.hpp since your last test run. Run the test "
                               "action on your current version before you submit.")
                    elif tool == "submit":
                        submitted = True
                        break
                    elif tool == "view":
                        obs = view(files, targs)
                    elif tool == "edit":
                        obs = edit(files, targs)
                        edited_since_test = edited_since_test or obs.startswith("edited")
                    elif tool == "test":
                        obs = test_obs(check(run_dir, files["kernel.hpp"]))
                        edited_since_test = False
                    else:
                        obs = f"error: unknown tool {tool!r}"
                messages.append({"role": "user", "content": obs})
            res = check(run_dir, files["kernel.hpp"])
            rec["rounds"].append({"round": rnd, "submitted": submitted, "steps": rec["steps"],
                                  "F": res.get("F", 0), "R": res.get("R", 0), "result": res,
                                  "kernel": files["kernel.hpp"]})
            if rec.get("context_full") or cond != "C2" or not (res.get("F") == 1 and res.get("R") == 0) \
                    or rnd == max_rounds - 1:
                break
            messages.append({"role": "user", "content": FEEDBACK_C2.format(
                report=alloc_report(res), round=rnd + 1, max_rounds=MAX_REPAIR_ROUNDS)})
    last = rec["rounds"][-1]
    rec.update({"solver": out.parents[2].name, "F": last["F"], "R": last["R"],
                "repair_rounds": len(rec["rounds"]) - 1, "seconds": round(time.time() - rec["started"], 1),
                "messages": messages})
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(rec, indent=1))
    return rec


def run_one(task: dict, cond: str, sample: int, policy, out: Path, max_steps: int) -> dict:
    """Drive one run with an HTTP endpoint (scripts/agent.py's Endpoint)."""
    g = run_steps(task, cond, sample, out, max_steps)
    try:
        msgs, seed = next(g)
        while True:
            try:
                got = policy(msgs, seed=seed)
            except ContextFull:
                got = None
            msgs, seed = g.send(got)
    except StopIteration as stop:
        return stop.value


def run_lockstep(jobs: list, root: Path, a) -> None:
    """All runs together with the model in this process (no server): each tick, one batched model call
    answers every waiting run, then the runs execute their actions (compiles) in parallel threads."""
    sys.path.insert(0, str(ROOT / "synthetic" / "generator"))
    from llm_batch import InProcessLLM  # noqa: E402
    llm = InProcessLLM(a.model, int(os.environ.get("GEN_MAX_MODEL_LEN", "32768")),
                       float(os.environ.get("GEN_GPU_MEM", "0.90")))
    queue = list(jobs)
    active = {}  # index -> [generator, (messages, seed), label]
    done = 0
    t0 = time.time()

    def start(job):
        t, c, s = job
        g = run_steps(t, c, s, root / c / t["id"] / f"sample_{s}.json", a.max_steps)
        return [g, next(g), f"{t['id']} {c} s{s}"]

    def advance(item, got):
        g = item[0]
        try:
            item[1] = g.send(got)
            return None
        except StopIteration as stop:
            return stop.value
        except Exception as e:  # noqa: BLE001  record and drop the run; rerun the job to retry it
            return {"error": repr(e)[:200]}

    nxt = 0
    with cf.ThreadPoolExecutor(a.workers) as ex:
        while queue or active:
            while queue and len(active) < a.batch:
                active[nxt] = start(queue.pop(0))
                nxt += 1
            keys = list(active)
            res = llm.generate([active[k][1][0] for k in keys], [active[k][1][1] for k in keys],
                               1.0, 0.95, 64, a.max_tokens)
            gots = [None if r["finish_reason"] == "too_long" else (r["text"], r["usage"]) for r in res]
            for k, rec in zip(keys, ex.map(lambda kg: advance(active[kg[0]], kg[1]), zip(keys, gots))):
                if rec is not None:
                    done += 1
                    msg = (f"ERROR {rec['error']}" if "error" in rec else
                           f"F={rec['F']} R={rec['R']} steps={rec['steps']}")
                    print(f"[{done}/{len(jobs)}] {active[k][2]}: {msg} ({time.time() - t0:.0f} s)", flush=True)
                    del active[k]


def cmd_run(a) -> None:
    tasks = tasks_from(Path(a.cands))
    if a.kinds:
        tasks = [t for t in tasks if t["kind"] in a.kinds.split(",")]
    jobs = [(t, c, s) for t in tasks for c in a.conditions.split(",") for s in range(a.samples)]
    if a.shard:
        k, n = map(int, a.shard.split("/"))
        jobs = [j for j in jobs if int(hashlib.sha256(f"{j[0]['id']}/{j[1]}/{j[2]}".encode())
                                       .hexdigest()[:8], 16) % n == k]
    if a.limit:
        jobs = jobs[:a.limit]
    random.Random(0).shuffle(jobs)  # mix long and short tasks
    tag = a.model.split("/")[-1]
    root = Path(a.out) / tag
    todo = [(t, c, s) for t, c, s in jobs if not (root / c / t["id"] / f"sample_{s}.json").exists()]
    print(f"{len(tasks)} tasks, {len(jobs)} runs, {len(jobs) - len(todo)} already done", flush=True)
    if a.offline:
        run_lockstep(todo, root, a)
        return
    policy = Endpoint(a.base_url, a.model, a.api_key, 1.0, 0.95, 64, a.max_tokens)
    n = 0

    def one(job):
        nonlocal n
        t, c, s = job
        try:
            r = run_one(t, c, s, policy, root / c / t["id"] / f"sample_{s}.json", a.max_steps)
            msg = f"F={r['F']} R={r['R']} steps={r['steps']} {r['seconds']:.0f}s"
        except Exception as e:  # noqa: BLE001  keep going; the run is retried on the next start
            msg = f"ERROR {e!r}"[:200]
        with LOCK:
            n += 1
            print(f"[{n}/{len(todo)}] {t['id']} {c} s{s}: {msg}", flush=True)

    with cf.ThreadPoolExecutor(a.workers) as ex:
        list(ex.map(one, todo))


GEN_TAG = {"google/gemma-4-31B-it-qat-w4a16-ct": "31B", "google/gemma-4-12B-it-qat-w4a16-ct": "12B"}
SOLVER_TAG = {"gemma-4-31B-it-qat-w4a16-ct": "31B", "gemma-4-12B-it-qat-w4a16-ct": "12B"}


def score_of(r: dict) -> float:
    """Implement: the kernel is correct and allocation-free (F*R). Remediate: the variant was fixed (F*R)."""
    return r["F"] * r["R"]


def matched(runs: list[dict]) -> list[dict]:
    """Runs on kernels from (domain, level) cells that hold accepted kernels of both generators."""
    gens = defaultdict(set)
    for r in runs:
        gens[(r["domain"], r["level"])].add(GEN_TAG.get(r["generator"], r["generator"]))
    return [r for r in runs if len(gens[(r["domain"], r["level"])]) == 2]


def did(runs: list[dict], n_boot: int = 2000, seed: int = 0) -> tuple[float, float, float, int] | None:
    """Self-preference as a difference in differences of the score, in points:
    (31B on 31B kernels - 31B on 12B kernels) - (12B on 31B kernels - 12B on 12B kernels).
    Positive: each model does better on its own kernels than the other model does. The second difference
    removes any difference in difficulty between the two halves. 95% bootstrap CI over kernels, resampled
    within each generator half. None if a cell is empty."""
    by = defaultdict(lambda: defaultdict(list))  # generator -> kernel -> [(solver, score)]
    for r in runs:
        g, sv = GEN_TAG.get(r["generator"]), SOLVER_TAG.get(r.get("solver"), r.get("solver"))
        if g and sv:
            by[g][r["kernel"]].append((sv, score_of(r)))

    def est(kernels: dict[str, list]) -> float | None:
        cell = defaultdict(list)
        for g, ks in kernels.items():
            for k in ks:
                for sv, sc in by[g][k]:
                    cell[(sv, g)].append(sc)
        if any(not cell[(sv, g)] for sv in ("31B", "12B") for g in ("31B", "12B")):
            return None
        m = {k: sum(v) / len(v) for k, v in cell.items()}
        return 100 * ((m[("31B", "31B")] - m[("31B", "12B")]) - (m[("12B", "31B")] - m[("12B", "12B")]))

    base = {g: list(by[g]) for g in ("31B", "12B")}
    point = est(base)
    if point is None:
        return None
    rng = random.Random(seed)
    boots = sorted(x for x in (est({g: [rng.choice(ks) for _ in ks] for g, ks in base.items()})
                               for _ in range(n_boot)) if x is not None)
    lo, hi = boots[int(0.025 * len(boots))], boots[int(0.975 * len(boots)) - 1]
    return point, lo, hi, sum(len(v) for v in base.values())


def cmd_report(a) -> None:
    runs = [json.loads(p.read_text()) for p in sorted(Path(a.runs).glob("*/*/*/sample_*.json"))]
    for r in runs:
        r["solver"] = SOLVER_TAG.get(r.get("solver"), r.get("solver", "?"))
    # 1. rates per solver, condition, kind and subset
    groups = defaultdict(list)
    for r in runs:
        for key in [("all",), ("level", r["level"]), ("generator", GEN_TAG.get(r["generator"], r["generator"]))]:
            groups[(r["solver"], r["condition"], r["kind"]) + key].append(r)
    print("## 1. Rates (remediate is the headline; implement tasks were solved once by both models during "
          "generation, G1, so their rates are inflated by construction)\n")
    print("| Solver | Cond. | Kind | Subset | Runs | Resolved | RT-safe resolved |")
    print("|---|---|---|---|---|---|---|")
    for k in sorted(groups, key=lambda k: (k[2] != "remediate", k)):
        rs = groups[k]
        f = 100 * sum(r["F"] for r in rs) / len(rs)
        fr = 100 * sum(r["F"] * r["R"] for r in rs) / len(rs)
        print(f"| {k[0]} | {k[1]} | {k[2]} | {' '.join(k[3:])} | {len(rs)} | {f:.0f} | {fr:.0f} |")
    # 2. self-preference, all kernels and the matched subset
    print("\n## 2. Self-preference: difference in differences of the RT-safe resolved rate, points "
          "(positive = models do better on their own kernels), 95% bootstrap CI over kernels\n")
    print("| Kind | Cond. | Level | Kernels | All kernels | Matched subset |")
    print("|---|---|---|---|---|---|")
    levels = sorted({r["level"] for r in runs}) + ["all"]
    for kind in ("remediate", "implement"):
        for cond in sorted({r["condition"] for r in runs}):
            for lv in levels:
                sub = [r for r in runs if r["kind"] == kind and r["condition"] == cond and lv in ("all", r["level"])]
                full, mat = did(sub), did(matched(sub))
                fmt = lambda d: "--" if d is None else f"{d[0]:+.0f} [{d[1]:+.0f}, {d[2]:+.0f}]"  # noqa: E731
                if full or mat:
                    print(f"| {kind} | {cond} | {lv} | {full[3] if full else 0} | {fmt(full)} | {fmt(mat)} |")
    # 3. CIs over kernels, paired differences against C0, the effect of the report within C2, run-to-run noise
    print("\n## 3. Outcome shares with 95% bootstrap CIs over kernels; paired differences to C0 "
          "(sign-flip test over kernels)\n")
    print("| Solver | Kind | Cond. | Runs | Clean | Still allocates | Tests fail | Diff. to C0 |")
    print("|---|---|---|---|---|---|---|---|")
    fmt = lambda t: f"{t[0]:.0f} [{t[1]:.0f}, {t[2]:.0f}]"  # noqa: E731
    for sv in ("31B", "12B"):
        for kind in ("implement", "remediate"):
            for cond in ("C0", "C1", "C2"):
                rs = [r for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == cond]
                if not rs:
                    continue
                shares = [kernel_ci(rs, f) for f in (lambda r: r["F"] * r["R"], lambda r: r["F"] * (1 - r["R"]),
                                                     lambda r: 1 - r["F"])]
                d = paired(runs, sv, kind, cond) if cond != "C0" else None
                dtxt = "" if d is None else f"{d[0]:+.0f} [{d[1]:+.0f}, {d[2]:+.0f}], p={d[3]:.3f}"
                print(f"| {sv} | {kind} | {cond} | {len(rs)} | " + " | ".join(fmt(x) for x in shares) + f" | {dtxt} |")
    print("\nC2 round 0 has the same prompt as C1, so C1 against C2 round 0 measures run-to-run noise; "
          "C2 round 0 against C2 final measures the report.\n")
    print("| Solver | Kind | C1 | C2 round 0 | C2 final | Runs that differ C1 vs C2 round 0 | Reports | Clean after |")
    print("|---|---|---|---|---|---|---|---|")
    for sv in ("31B", "12B"):
        for kind in ("implement", "remediate"):
            c1 = {r["id"]: r for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == "C1"}
            c2 = [r for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == "C2"]
            if not c1 or not c2:
                continue
            rep = [r for r in c2 if r.get("repair_rounds", 0) > 0]
            diff = sum(score_of(c1[r["id"]]) != first_round(r) for r in c2 if r["id"] in c1)
            print(f"| {sv} | {kind} | {pct(c1.values())} | {100 * sum(map(first_round, c2)) / len(c2):.0f} | "
                  f"{pct(c2)} | {diff} of {len(c2)} | {len(rep)} | {sum(map(score_of, rep))} |")
    if a.latex:
        write_latex(runs, Path(a.latex))
        print(f"\nwrote {a.latex}")


def pct(rs) -> str:
    rs = list(rs)
    return f"{100 * sum(map(score_of, rs)) / len(rs):.0f}"


def first_round(r: dict) -> int:
    rounds = r.get("rounds")
    if isinstance(rounds, str):
        import ast
        rounds = ast.literal_eval(rounds)
    return rounds[0]["F"] * rounds[0]["R"] if rounds else 0


def kernel_ci(rs: list[dict], f, n_boot: int = 4000, seed: int = 0) -> tuple[float, float, float]:
    """Share of runs with f(r) = 1 in percent, 95% bootstrap CI resampling kernels (a kernel's variants are
    not independent)."""
    by = defaultdict(list)
    for r in rs:
        by[r["kernel"]].append(f(r))
    ks, rng = list(by), random.Random(seed)
    share = lambda kk: 100 * sum(sum(by[k]) for k in kk) / sum(len(by[k]) for k in kk)  # noqa: E731
    boots = sorted(share([rng.choice(ks) for _ in ks]) for _ in range(n_boot))
    return share(ks), boots[int(0.025 * n_boot)], boots[int(0.975 * n_boot) - 1]


def paired(runs: list[dict], sv: str, kind: str, cond: str, n_perm: int = 20000, seed: int = 1):
    """Mean per-task difference of the score, cond minus C0, in points; 95% bootstrap CI over kernels and a
    two-sided sign-flip test over kernels (per-kernel sums)."""
    base = {r["id"]: score_of(r) for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == "C0"}
    by = defaultdict(list)
    for r in runs:
        if r["solver"] == sv and r["kind"] == kind and r["condition"] == cond and r["id"] in base:
            by[r["kernel"]].append(score_of(r) - base[r["id"]])
    if not by:
        return None
    pt, lo, hi = kernel_ci([{"kernel": k, "d": d} for k, ds in by.items() for d in ds], lambda r: r["d"])
    sums, rng = [sum(v) for v in by.values()], random.Random(seed)
    obs = abs(sum(sums))
    p = sum(abs(sum(x if rng.random() < 0.5 else -x for x in sums)) >= obs - 1e-9 for _ in range(n_perm)) / n_perm
    return pt, lo, hi, p


def write_latex(runs: list[dict], path: Path) -> None:
    """Paper table: share of clean runs (tests pass, no allocation) per solver, task kind and condition with
    95% CIs over kernels, and the self-preference difference in differences per condition."""
    # value and interval in separate columns (r and l), so estimates align on their units and intervals on
    # their brackets, also for signed values of different width
    grey = lambda x: f"{{\\scriptsize\\textcolor{{black!55}}{{{x}}}}}"  # noqa: E731
    # interval bounds padded to two digits with \phantom, so commas and brackets line up in the column
    pad = lambda v: "\\phantom{0}" * max(0, 2 - len(str(abs(round(v))))) + f"{v:.0f}"  # noqa: E731
    spad = lambda v: "\\phantom{0}" * max(0, 2 - len(str(abs(round(v))))) + f"${v:+.0f}$"  # noqa: E731
    ci = lambda t: f"{t[0]:.0f} & {grey(f'[{pad(t[1])}, {pad(t[2])}]')}"  # noqa: E731
    sg = lambda t: (f"${t[0]:+.0f}$ & {grey(f'[{spad(t[1])}, {spad(t[2])}]')}"
                    if t else "-- &")  # noqa: E731
    conds = ("C0", "C1", "C2")
    lines = ["% generated by scripts/agent_synthetic.py report --latex; do not edit by hand",
             "\\begin{tabular}{@{}llr" + "r@{\\,}l" * 3 + "@{}}", "  \\toprule",
             "  Solver & Task & Runs & " + " & ".join(f"\\multicolumn{{2}}{{c}}{{{c}}}" for c in ("C0", "C1", "C2"))
             + " \\\\", "  \\midrule"]
    for si, sv in enumerate(("31B", "12B")):
        for ki, kind in enumerate(("implement", "remediate")):
            cells = []
            for cond in conds:
                rs = [r for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == cond]
                cells.append(ci(kernel_ci(rs, score_of)) if rs else "-- &")
            n = sum(1 for r in runs if r["solver"] == sv and r["kind"] == kind and r["condition"] == "C0")
            lines.append(f"  {'Gemma~4 ' + sv if ki == 0 else ''} & {kind} & {n} & " + " & ".join(cells) + " \\\\")
        lines.append("  \\midrule")
    for ki, kind in enumerate(("implement", "remediate")):
        cells = [sg(did([r for r in runs if r["kind"] == kind and r["condition"] == c])) for c in conds]
        lines.append(f"  {'Self-pref.' if ki == 0 else ''} & {kind} & & " + " & ".join(cells) + " \\\\")
    lines += ["  \\bottomrule", "\\end{tabular}"]
    path.write_text("\n".join(lines) + "\n")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("--cands", required=True, help="candidates directory of a synthetic/generator/generate.py run")
    r.add_argument("--model", required=True)
    r.add_argument("--base-url", default=os.environ.get("RTSWE_BASE_URL", "http://localhost:8000/v1"))
    r.add_argument("--api-key", default=os.environ.get("RTSWE_API_KEY", "none"))
    r.add_argument("--conditions", default="C0,C1,C2")
    r.add_argument("--kinds", default="", help="implement,remediate (default: both)")
    r.add_argument("--samples", type=int, default=1)
    r.add_argument("--max-steps", type=int, default=20, help="actions per round")
    r.add_argument("--max-tokens", type=int, default=4096)
    r.add_argument("--workers", type=int, default=12)
    r.add_argument("--shard", default="", help="k/n: this job takes the runs with hash %% n == k")
    r.add_argument("--limit", type=int, default=0)
    r.add_argument("--out", default=str(ROOT / "results/synthetic_runs"))
    r.add_argument("--offline", action="store_true", default=os.environ.get("GEN_LLM") == "offline",
                   help="model in this process (synthetic/generator/llm_batch.py), no server; all runs advance in lockstep")
    r.add_argument("--batch", type=int, default=64, help="offline: runs in flight at once")
    rp = sub.add_parser("report")
    rp.add_argument("--runs", default=str(ROOT / "results/synthetic_runs"))
    rp.add_argument("--latex", help="also write the paper table (e.g. paper/tables/tier1_solve.tex)")
    a = ap.parse_args()
    {"run": cmd_run, "report": cmd_report}[a.cmd](a)


if __name__ == "__main__":
    main()
