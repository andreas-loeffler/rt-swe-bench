#!/usr/bin/env python3
"""Summarize agent runs (results/runs/<model>/<condition>/<task>/sample_<k>.json).

Per model and condition:
  Pass@1_func  mean over tasks of mean(F) over samples
  Pass@1_RT    mean over tasks of mean(F*R) over samples
  Track 1: gap = Pass@1_func - Pass@1_RT
  Track 2: remediation = Pass@1_RT, regression = mean(F == 0)
  C2: mean repair rounds used; "C2-best" scores each C2 run by its best round
  instead of its last submitted patch
Then a per-task matrix of F/R for every run.

    python3 scripts/summarize_runs.py [results/runs] [--tasks tasks]
"""
import argparse
import json
from collections import defaultdict
from pathlib import Path


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("runs", nargs="?", default="results/runs")
    ap.add_argument("--tasks", default="tasks")
    a = ap.parse_args()

    track = {p.parent.name: json.loads(p.read_text())["track"] for p in Path(a.tasks).glob("*/task.json")}
    runs = defaultdict(lambda: defaultdict(list))  # (model, cond) -> task -> [record]
    for f in sorted(Path(a.runs).glob("*/*/*/sample_*.json")):
        model, cond, task = f.parts[-4], f.parts[-3], f.parts[-2]
        if model == "replay":
            continue
        rec = json.loads(f.read_text())
        runs[(model, cond)][task].append(rec)
        if cond == "C2" and rec.get("rounds"):
            # Secondary C2 score: the best round instead of the last submitted one.
            best = max(rec["rounds"], key=lambda x: (x["F"] * x["R"], x["F"]))
            runs[(model, "C2-best")][task].append({**rec, "F": best["F"], "R": best["R"]})

    print("| Model | Cond. | Tasks | Runs | Pass@1 func | Pass@1 RT | T1 gap | T2 remed. | T2 regr. | Repair rounds |")
    print("|---|---|---|---|---|---|---|---|---|---|")
    for (model, cond), by_task in sorted(runs.items()):
        f_task = {t: sum(r["F"] for r in rs) / len(rs) for t, rs in by_task.items()}
        fr_task = {t: sum(r["F"] * r["R"] for r in rs) / len(rs) for t, rs in by_task.items()}
        n = len(by_task)
        pf = sum(f_task.values()) / n
        pr = sum(fr_task.values()) / n
        t1 = [t for t in by_task if track.get(t) == 1]
        t2 = [t for t in by_task if track.get(t) == 2]
        gap = (sum(f_task[t] - fr_task[t] for t in t1) / len(t1)) if t1 else None
        remed = (sum(fr_task[t] for t in t2) / len(t2)) if t2 else None
        regr = (sum(1 - f_task[t] for t in t2) / len(t2)) if t2 else None
        rounds = [r.get("repair_rounds", 0) for rs in by_task.values() for r in rs]
        if cond == "C2-best":
            rounds = []
        fmt = lambda x: "n/a" if x is None else f"{100 * x:.0f}%"
        print(f"| {model} | {cond} | {n} | {sum(len(v) for v in by_task.values())} | {fmt(pf)} | {fmt(pr)} | "
              f"{fmt(gap)} | {fmt(remed)} | {fmt(regr)} | "
              f"{(sum(rounds) / len(rounds)) if cond == 'C2' and rounds else 0:.1f} |")

    print("\nPer run: F/R of the scored patch (C2: per round), steps, seconds\n")
    for (model, cond), by_task in sorted(runs.items()):
        if cond == "C2-best":
            continue
        for task, rs in sorted(by_task.items()):
            for r in rs:
                rounds = " -> ".join(f"{x['F']}/{x['R']}" for x in r["rounds"])
                print(f"{model:28s} {cond} {task} s{r['sample']}  {rounds:22s} steps={r['steps']:3d} "
                      f"{r.get('seconds', 0):6.0f}s{'  context full' if r.get('context_full') else ''}")


if __name__ == "__main__":
    main()
