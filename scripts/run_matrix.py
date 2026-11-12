#!/usr/bin/env python3
"""Run the evaluation matrix: tasks x conditions x samples, on N parallel workers.

Each job is one scripts/agent.py run; finished runs (result file present) are
skipped, so the matrix can be restarted at any time. With the native backend,
worker 0 uses /ws and worker k uses /ws_{k+1} (create them with
scripts/add_native_ws.sh); with Docker every run gets its own container.

    scripts/run_matrix.py --conditions C0,C1,C2,C3 --samples 3 --workers 3 \
        --model google/gemma-4-31B-it-qat-w4a16-ct --base-url http://localhost:8000/v1
    scripts/run_matrix.py --tasks rt-001,rt-002 --samples 1 --workers 1 --dry-run
"""
from __future__ import annotations

import argparse
import json
import os
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def result_path(model: str, cond: str, task: str, sample: int) -> Path:
    tag = model.split("/")[-1] + ("-thinking" if os.environ.get("RTSWE_THINKING") == "1" else "")
    return ROOT / "results/runs" / tag / cond / task / f"sample_{sample}.json"


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tasks", help="comma-separated task ids (default: every tasks/rt-*)")
    ap.add_argument("--conditions", default="C0,C1,C2,C3")
    ap.add_argument("--samples", type=int, default=3)
    ap.add_argument("--first-sample", type=int, default=0,
                    help="skip samples below this (e.g. 3: only add a 4th sample to runs made elsewhere)")
    ap.add_argument("--workers", type=int, default=1)
    ap.add_argument("--model", default="google/gemma-4-31B-it-qat-w4a16-ct")
    ap.add_argument("--base-url", default=os.environ.get("RTSWE_BASE_URL", "http://localhost:8000/v1"))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    tasks = a.tasks.split(",") if a.tasks else sorted(p.name for p in (ROOT / "tasks").glob("rt-*"))
    conds = a.conditions.split(",")
    # sample-major order: every task gets its first sample before any gets a second
    jobs = [(t, c, s) for s in range(a.first_sample, a.samples) for t in tasks for c in conds
            if not result_path(a.model, c, t, s).exists()]
    print(f"{len(jobs)} runs to do ({len(tasks)} tasks x {len(conds)} conditions x {a.samples} samples, "
          f"finished runs skipped), {a.workers} workers")
    if a.dry_run:
        for j in jobs:
            print("  ", *j)
        return

    backend = os.environ.get("RTSWE_BACKEND", "docker")
    log_dir = result_path(a.model, "x", "x", 0).parents[2] / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    q: queue.Queue = queue.Queue()
    for j in jobs:
        q.put(j)
    lock = threading.Lock()
    done = [0]
    t_start = time.time()

    def worker(k: int) -> None:
        env = dict(os.environ)
        if backend == "native":
            env["RTSWE_WS"] = "/ws" if k == 0 else f"/ws_{k + 1}"
            # one DDS domain per worker: the package tests of parallel workers on one host
            # otherwise discover each other's nodes and fail (e.g. joint_trajectory_controller
            # action tests: 0 failures alone, about 100 with four workers)
            env["ROS_DOMAIN_ID"] = str(k + 1)
        while True:
            try:
                task, cond, sample = q.get_nowait()
            except queue.Empty:
                return
            log = log_dir / f"{task}_{cond}_s{sample}.log"
            cmd = [sys.executable, str(ROOT / "scripts/agent.py"), "--task", str(ROOT / "tasks" / task),
                   "--condition", cond, "--sample", str(sample), "--model", a.model, "--base-url", a.base_url]
            t0 = time.time()
            with open(log, "w") as f:
                code = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, env=env).returncode
            res = result_path(a.model, cond, task, sample)
            verdict = "missing"
            if res.exists():
                r = json.loads(res.read_text())
                verdict = " -> ".join(f"{x['F']}/{x['R']}" for x in r["rounds"])
            with lock:
                done[0] += 1
                print(f"[{done[0]}/{len(jobs)}] w{k} {task} {cond} s{sample}: {verdict} "
                      f"({time.time() - t0:.0f} s, exit {code}, {(time.time() - t_start) / 60:.0f} min total)",
                      flush=True)

    threads = [threading.Thread(target=worker, args=(k,)) for k in range(a.workers)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()


if __name__ == "__main__":
    main()
