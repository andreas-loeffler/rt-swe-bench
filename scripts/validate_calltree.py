#!/usr/bin/env python3
"""Static call tree vs. the dynamic verifier, per task (paper: C3 side result).

Runs inside the benchmark container with the repository mounted at /rtswe:

    docker run --rm -v "$PWD":/rtswe:ro rt-swe-bench/base:jazzy-d71703b \
        python3 /rtswe/scripts/validate_calltree.py > results/calltree/validation.jsonl

For every task, on the code that allocates (Track 2: the task's starting point;
Track 1: starting point plus the naive patch) and on the reference fix:
  fix_hit   the call tree marks a line that the fix changes (Track 2: lines the
            reference removes or edits; Track 1: lines the naive patch adds)
  dyn       malloc-guard's allocation sites in package code (from validation.jsonl)
            and how many of them the call tree marks
  lines     package lines the call tree marks as on an allocation path (noise)
"""
import json
import re
import subprocess
import sys
from pathlib import Path

RTSWE = Path("/rtswe")
REPO = "/ws/src/ros2_controllers"
CT = RTSWE / "harness/calltree/calltree.py"


def sh(cmd: str) -> str:
    return subprocess.run(["bash", "-c", cmd], capture_output=True, text=True, check=True).stdout


def pinned() -> str:
    for line in Path("/opt/rtswe/source-manifest.txt").read_text().splitlines():
        if line.startswith("ros2_controllers="):
            return line.split("=", 1)[1]
    raise SystemExit("no pinned commit")


def reset(sha: str) -> None:
    sh(f"cd {REPO} && git reset -q --hard {sha} && git clean -qfd")


def apply(diff: Path) -> None:
    sh(f"cd {REPO} && git apply --whitespace=nowarn {diff}")


def calltree(package: str) -> dict:
    out = subprocess.run(["python3", str(CT), "--package", package, "--json"],
                         capture_output=True, text=True)
    if out.returncode != 0:
        raise RuntimeError(out.stdout[-1500:] + out.stderr[-1500:])
    return json.loads(out.stdout)


def changed_lines(diff: str, side: str) -> set[str]:
    """side='old': lines the diff removes (or the line before a pure insertion);
    side='new': lines it adds. As 'path:line' relative to the repository."""
    out, path, o, n = set(), None, 0, 0
    for line in diff.splitlines():
        if line.startswith("+++ "):
            path = line[6:] if line.startswith("+++ b/") else None
        elif line.startswith("@@"):
            m = re.match(r"@@ -(\d+)(?:,\d+)? \+(\d+)(?:,\d+)? @@", line)
            o, n = int(m.group(1)), int(m.group(2))
        elif path is None or line.startswith(("---", "diff ", "index ")):
            continue
        elif line.startswith("-"):
            if side == "old":
                out.add(f"{path}:{o}")
            o += 1
        elif line.startswith("+"):
            if side == "new":
                out.add(f"{path}:{n}")
            elif side == "old":
                out.add(f"{path}:{max(o - 1, 1)}")
            n += 1
        else:
            o, n = o + 1, n + 1
    return out


def near(lines: set[str], targets: set[str], tol: int = 1) -> set[str]:
    have = {}
    for x in lines:
        f, l = x.rsplit(":", 1)
        have.setdefault(f, set()).add(int(l))
    hit = set()
    for t in targets:
        f, l = t.rsplit(":", 1)
        if any(abs(int(l) - k) <= tol for k in have.get(f, ())):
            hit.add(t)
    return hit


def dynamic_sites(task_dir: Path, variant: str) -> set[str]:
    for line in (task_dir / "validation.jsonl").read_text().splitlines():
        v = json.loads(line)
        if v.get("variant") == variant:
            sites = (v.get("alloc") or {}).get("sites", [])
            return {f"{m.group(1)}:{m.group(2)}" for s in sites
                    for m in [re.search(r"ros2_controllers/([^:]+):(\d+)", s["site"])] if m}
    return set()


def main() -> None:
    sha = pinned()
    for tdir in sorted((RTSWE / "tasks").glob("rt-*")):
        task = json.loads((tdir / "task.json").read_text())
        pkg, track = task["package"], task["track"]
        rec = {"task": task["id"], "track": track, "package": pkg}
        try:
            reset(sha)
            if task.get("base_patch"):
                apply(tdir / task["base_patch"])
            if track == 1:
                apply(tdir / "naive.diff")
                targets = changed_lines((tdir / "naive.diff").read_text(), "new")
                dyn = dynamic_sites(tdir, "naive")
            else:
                targets = changed_lines((tdir / "reference.diff").read_text(), "old")
                dyn = dynamic_sites(tdir, "unpatched")
            ct = calltree(pkg)
            lines = set(ct["lines"]) | set(ct["logging"])  # logging calls are allocation sites too
            rec.update({
                "targets": sorted(targets), "fix_hit": sorted(near(lines, targets)),
                "dyn": sorted(dyn), "dyn_marked": sorted(near(lines, dyn, tol=0)),
                "lines": len(lines), "sites": len(ct["sites"]), "logging": len(ct["logging"]),
                "throw_sites": ct["throw_sites"],
            })
            reset(sha)
            if task.get("base_patch"):
                apply(tdir / task["base_patch"])
            apply(tdir / "reference.diff")
            ref = calltree(pkg)
            rec.update({"ref_lines": len(ref["lines"]), "ref_sites": len(ref["sites"])})
        except Exception as e:  # noqa: BLE001 - report and continue with the next task
            rec["error"] = str(e)[-800:]
        print(json.dumps(rec), flush=True)
        print(f"{rec['task']}: fix_hit {len(rec.get('fix_hit', []))}/{len(rec.get('targets', []))} "
              f"dyn {len(rec.get('dyn_marked', []))}/{len(rec.get('dyn', []))} lines {rec.get('lines')} "
              f"-> ref {rec.get('ref_lines')} {rec.get('error', '')[:200]}", file=sys.stderr)
    reset(sha)


if __name__ == "__main__":
    main()
