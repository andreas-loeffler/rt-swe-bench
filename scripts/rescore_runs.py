#!/usr/bin/env python3
"""Re-score the patches of finished agent runs with scripts/evaluate.sh (default: Docker, the pinned image)
and compare the verdicts with the ones recorded during the run (e.g. on the native Colab backend).

For every run it scores the patch of the scored round (the last one); for C2 runs whose best round
differs from the last, the best round too. Results go to one JSONL file, one line per patch; patches
already in that file are skipped, so an interrupted re-scoring can be restarted.

    python3 scripts/rescore_runs.py RUNS_DIR --out results/rescore/<model>.jsonl [--models <model>] [--workers 3]
    python3 scripts/rescore_runs.py RUNS_DIR --out ... --report    # only print the comparison
"""
import argparse
import json
import os
import subprocess
import tempfile
import threading
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
# slowest packages first, so the long jobs do not end up last
SLOW = {"rt-010": 0, "rt-014": 1, "rt-003": 2}


def jobs(runs_dir: Path, models: set[str] | None = None):
    for f in sorted(runs_dir.glob("*/*/*/sample_*.json")):
        model, cond, task = f.parts[-4], f.parts[-3], f.parts[-2]
        if model == "replay" or (models and model not in models):
            continue
        rec = json.loads(f.read_text())
        rounds = rec.get("rounds") or []
        if not rounds:
            continue
        picks = {len(rounds) - 1}
        if cond in ("C2", "C2B", "C2N"):
            best = max(range(len(rounds)), key=lambda i: (rounds[i]["F"] * rounds[i]["R"], rounds[i]["F"]))
            picks.add(best)
        for i in sorted(picks):
            rd = rounds[i]
            yield {"id": f"{model}/{cond}/{task}/s{rec['sample']}/r{i}", "model": model, "cond": cond,
                   "task": task, "sample": rec["sample"], "round": i, "patch": rd.get("patch") or "",
                   "recorded": {"F": rd["F"], "R": rd["R"], "result": rd.get("result") or {}}}


def score(job, log_root: Path):
    log_dir = log_root / job["id"].replace("/", "__")
    log_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", suffix=".diff", delete=False) as p:
        p.write(job["patch"])
    patch = p.name if job["patch"].strip() else "none"
    try:
        r = subprocess.run(["bash", "scripts/evaluate.sh", f"tasks/{job['task']}", patch], cwd=ROOT,
                           capture_output=True, text=True, timeout=3 * 3600,
                           env={**os.environ, "LOG_DIR": str(log_dir)})
        out = json.loads(r.stdout.strip().splitlines()[-1]) if r.stdout.strip() else {"error": r.stderr[-2000:]}
    except Exception as e:  # noqa: BLE001  record and continue
        out = {"error": repr(e)}
    finally:
        os.unlink(p.name)
    if out.get("logs"):  # recorded relative to the repository, not as this machine's absolute path
        out["logs"] = os.path.relpath(out["logs"], ROOT)
    return {k: v for k, v in job.items() if k != "patch"} | {"rescored": out}


def report(out_file: Path):
    rows = [json.loads(l) for l in out_file.read_text().splitlines() if l.strip()]
    same = [r for r in rows if "error" not in r["rescored"]
            and (r["rescored"].get("F"), r["rescored"].get("R")) == (r["recorded"]["F"], r["recorded"]["R"])]
    errors = [r for r in rows if "error" in r["rescored"]]
    diff = [r for r in rows if r not in same and r not in errors]
    print(f"{len(rows)} patches re-scored: {len(same)} same verdict, {len(diff)} different, {len(errors)} errors")
    for r in diff:
        rs = r["rescored"]
        print(f"  {r['id']}: recorded {r['recorded']['F']}/{r['recorded']['R']}, now {rs.get('F')}/{rs.get('R')} "
              f"(applied {rs.get('applied')}, build {rs.get('build')}, {rs.get('tests', '')})")
    for r in errors:
        print(f"  {r['id']}: error {r['rescored']['error'][:200]}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("runs")
    ap.add_argument("--out", required=True)
    ap.add_argument("--workers", type=int, default=3)
    ap.add_argument("--logs", default="results/raw/rescore")
    ap.add_argument("--report", action="store_true")
    ap.add_argument("--models", default="", help="comma-separated model directory names (default: all)")
    a = ap.parse_args()
    out = Path(a.out)
    if a.report:
        report(out)
        return
    out.parent.mkdir(parents=True, exist_ok=True)
    done = {json.loads(l)["id"] for l in out.read_text().splitlines() if l.strip()} if out.exists() else set()
    todo = sorted((j for j in jobs(Path(a.runs), set(filter(None, a.models.split(",")))) if j["id"] not in done), key=lambda j: SLOW.get(j["task"], 9))
    print(f"{len(done)} already scored, {len(todo)} to go", flush=True)
    lock = threading.Lock()
    n = 0

    def one(job):
        nonlocal n
        res = score(job, Path(a.logs).resolve())
        with lock:
            n += 1
            with out.open("a") as fh:
                fh.write(json.dumps(res) + "\n")
            rs = res["rescored"]
            print(f"[{n}/{len(todo)}] {job['id']}: recorded {job['recorded']['F']}/{job['recorded']['R']}, "
                  f"docker {rs.get('F')}/{rs.get('R')}" + (" ERROR" if "error" in rs else ""), flush=True)

    with ThreadPoolExecutor(a.workers) as ex:
        list(ex.map(one, todo))
    report(out)


if __name__ == "__main__":
    main()
