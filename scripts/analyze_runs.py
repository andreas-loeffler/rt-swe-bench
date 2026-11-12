#!/usr/bin/env python3
"""Statistics for the evaluation matrix (results/runs/<model>/<condition>/<task>/sample_<k>.json).

Tasks are the unit of analysis: per task, a score is the mean over its samples, and confidence
intervals come from a bootstrap over tasks (resampling tasks with replacement), so the interval
reflects the 16 tasks, not the 48 runs. Paired comparisons between conditions use the per-task
difference, with a bootstrap interval and an exact sign-flip permutation test (all 2^n sign
assignments; n = number of tasks).

Sections:
  1. resolved rate (F=1) and RT-safe resolved rate (F=R=1) per model and condition, with 95% CIs, overall and per track
  2. Paired differences in the RT-safe resolved rate between conditions
  3. Outcome of the scored patch per condition (empty, no build, tests fail, allocates, clean)
  4. Track 2 runs that pass the tests but allocate: allocation unchanged, reduced, or moved
  5. C2 repair rounds: what the malloc-guard report led to, round by round
  6. Per-task RT-safe resolved rate
  7. Cost: steps, seconds, tokens

    python3 scripts/analyze_runs.py [results/runs] [--tasks tasks] [--json out.json] [--latex t.tex]
        [--rescore results/rescore/*.jsonl]
"""
import argparse
import itertools
import json
import random
from collections import Counter, defaultdict
from pathlib import Path

B = 10000  # bootstrap resamples
SEED = 20261103


def load_rescore(paths):
    """Verdicts from scripts/rescore_runs.py, keyed by model/cond/task/s<k>/r<i>."""
    ov = {}
    for p in paths:
        for line in Path(p).read_text().splitlines():
            j = json.loads(line)
            r = j.get("rescored") or {}
            if "F" in r and "R" in r:
                ov[j["id"]] = r
    return ov


def load(runs_dir: Path, rescore=None, max_samples=None):
    """Runs grouped by (model, cond) and task. With rescore, the re-scored rounds take those verdicts.
    max_samples: keep samples 0..max_samples-1 only (the main matrix has 3 per task and condition; the
    Track 2 ablation added samples 3-7 for C1 and C2)."""
    rescore = rescore or {}
    runs = defaultdict(lambda: defaultdict(list))  # (model, cond) -> task -> [record]
    for f in sorted(runs_dir.glob("*/*/*/sample_*.json")):
        model, cond, task = f.parts[-4], f.parts[-3], f.parts[-2]
        if model == "replay":
            continue
        rec = json.loads(f.read_text())
        if max_samples is not None and rec["sample"] >= max_samples:
            continue
        rounds = rec.get("rounds") or []
        for i, rd in enumerate(rounds):
            r = rescore.get(f"{model}/{cond}/{task}/s{rec['sample']}/r{i}")
            if r:
                rd.update(F=r["F"], R=r["R"], result=r)
        if rounds:
            rec["F"], rec["R"] = rounds[-1]["F"], rounds[-1]["R"]
        runs[(model, cond)][task].append(rec)
        if cond == "C2" and rec.get("rounds"):
            best = max(rec["rounds"], key=lambda x: (x["F"] * x["R"], x["F"]))
            runs[(model, "C2-best")][task].append({**rec, "F": best["F"], "R": best["R"], "best_of": rec})
    return runs


def per_task(by_task, fn):
    return {t: sum(fn(r) for r in rs) / len(rs) for t, rs in by_task.items()}


def boot_ci(values, rng=None):
    """Mean and 95% percentile bootstrap interval over the given per-task values.

    Each call uses its own generator with a fixed seed, so a number does not depend on what was
    computed before it."""
    rng = random.Random(SEED)
    v = list(values)
    if not v:
        return None, None, None
    n = len(v)
    means = sorted(sum(rng.choice(v) for _ in range(n)) / n for _ in range(B))
    return sum(v) / n, means[int(0.025 * B)], means[int(0.975 * B) - 1]


def sign_flip_p(diffs):
    """Exact two-sided permutation p-value for mean(diffs) != 0 under random sign flips."""
    d = [x for x in diffs if x != 0]
    if not d:
        return 1.0
    obs = abs(sum(d))
    hits = sum(1 for s in itertools.product((1, -1), repeat=len(d))
               if abs(sum(a * b for a, b in zip(s, d))) >= obs - 1e-12)
    return hits / 2 ** len(d)


def scored(rec):
    """The round whose patch was scored (C2-best: the best round)."""
    src = rec.get("best_of", rec)
    rounds = src.get("rounds") or []
    if not rounds:
        return {}
    if "best_of" in rec:
        return max(rounds, key=lambda x: (x["F"] * x["R"], x["F"]))
    return rounds[-1]


def outcome(rec):
    rd = scored(rec)
    res = rd.get("result") or {}
    if not (rd.get("patch") or "").strip():
        return "empty patch"
    if not res.get("applied") or not res.get("build"):
        return "no build"
    if rec["F"] == 0:
        return "tests fail, allocates" if rec["R"] == 0 else "tests fail, clean"
    return "tests pass, allocates" if rec["R"] == 0 else "tests pass, clean"


OUTCOMES = ["empty patch", "no build", "tests fail, allocates", "tests fail, clean",
            "tests pass, allocates", "tests pass, clean"]


def site_fns(alloc):
    return {s["site"].split(" (")[0] for s in (alloc or {}).get("sites", [])}


def fmt(x):
    return "n/a" if x is None else f"{100 * x:.0f}"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("runs", nargs="?", default="results/runs")
    ap.add_argument("--tasks", default="tasks")
    ap.add_argument("--json", help="also write all numbers to this file")
    ap.add_argument("--latex", help="also write the main results table (tabular) to this file")
    ap.add_argument("--max-samples", type=int, default=3,
                    help="samples per task and condition in the main matrix (0 = all)")
    ap.add_argument("--rescore", nargs="*", default=[],
                    help="JSONL files from scripts/rescore_runs.py whose verdicts replace the recorded ones")
    a = ap.parse_args()
    rng = random.Random(SEED)

    tasks = Path(a.tasks)
    track = {p.parent.name: json.loads(p.read_text())["track"] for p in tasks.glob("*/task.json")}
    base = {}  # task -> alloc of the unpatched task base
    for p in tasks.glob("*/validation.jsonl"):
        for line in p.read_text().splitlines():
            r = json.loads(line)
            if r.get("variant") == "unpatched":
                base[r["task"]] = r.get("alloc") or {}
    runs = load(Path(a.runs), load_rescore(a.rescore), a.max_samples or None)
    out = {"pass": [], "paired": [], "outcomes": {}, "track2_alloc": {}, "c2_rounds": {}, "per_task": {}, "cost": {}}

    print("## 1. Resolved and RT-safe resolved rate with 95% bootstrap CIs over tasks\n")
    print("| Model | Cond. | Subset | Tasks | Runs | Resolved | RT-safe resolved |")
    print("|---|---|---|---|---|---|---|")
    rt_by = {}  # (model, cond) -> task -> Pass@1 RT
    for (model, cond), by_task in sorted(runs.items()):
        f_t = per_task(by_task, lambda r: r["F"])
        fr_t = per_task(by_task, lambda r: r["F"] * r["R"])
        rt_by[(model, cond)] = fr_t
        for subset, keep in (("all", lambda t: True), ("Track 1", lambda t: track.get(t) == 1),
                             ("Track 2", lambda t: track.get(t) == 2)):
            ts = [t for t in by_task if keep(t)]
            if not ts:
                continue
            pf = boot_ci([f_t[t] for t in ts], rng)
            pr = boot_ci([fr_t[t] for t in ts], rng)
            n_runs = sum(len(by_task[t]) for t in ts)
            print(f"| {model} | {cond} | {subset} | {len(ts)} | {n_runs} | "
                  f"{fmt(pf[0])} [{fmt(pf[1])}, {fmt(pf[2])}] | {fmt(pr[0])} [{fmt(pr[1])}, {fmt(pr[2])}] |")
            out["pass"].append({"model": model, "cond": cond, "subset": subset, "tasks": len(ts), "runs": n_runs,
                                "func": pf, "rt": pr})

    print("\n## 2. Paired differences in the RT-safe resolved rate (row minus column condition, per task)\n")
    print("| Model | Comparison | Tasks | Mean diff | 95% CI | Tasks better / worse | p (sign-flip) |")
    print("|---|---|---|---|---|---|---|")
    models = sorted({m for m, _ in runs})
    pairs = [("C1", "C0"), ("C2", "C0"), ("C2-best", "C0"), ("C3", "C0"), ("C2", "C1"), ("C2", "C3"), ("C1", "C3")]
    for model in models:
        for x, y in pairs:
            if (model, x) not in rt_by or (model, y) not in rt_by:
                continue
            ts = sorted(set(rt_by[(model, x)]) & set(rt_by[(model, y)]))
            d = [rt_by[(model, x)][t] - rt_by[(model, y)][t] for t in ts]
            m, lo, hi = boot_ci(d, rng)
            p = sign_flip_p(d)
            better, worse = sum(v > 0 for v in d), sum(v < 0 for v in d)
            print(f"| {model} | {x} vs {y} | {len(ts)} | {100 * m:+.0f} | [{100 * lo:+.0f}, {100 * hi:+.0f}] | "
                  f"{better} / {worse} | {p:.3f} |")
            out["paired"].append({"model": model, "x": x, "y": y, "tasks": len(ts), "diff": [m, lo, hi],
                                  "better": better, "worse": worse, "p": p})

    print("\n## 3. Outcome of the scored patch (share of runs)\n")
    print("| Model | Cond. | Runs | Not submitted | " + " | ".join(OUTCOMES) + " |")
    print("|---|---|---|---|" + "---|" * len(OUTCOMES))
    for (model, cond), by_task in sorted(runs.items()):
        recs = [r for rs in by_task.values() for r in rs]
        c = Counter(outcome(r) for r in recs)
        unsub = sum(1 for r in recs if not scored(r).get("submitted", True))
        n = len(recs)
        print(f"| {model} | {cond} | {n} | {fmt(unsub / n)} | " + " | ".join(fmt(c[o] / n) for o in OUTCOMES) + " |")
        out["outcomes"][f"{model}|{cond}"] = {"runs": n, "not_submitted": unsub, **{o: c[o] for o in OUTCOMES}}

    print("\n## 4. Track 2 runs that pass the tests but still allocate, compared with the task base\n")
    print("| Model | Cond. | Runs | Unchanged | Fewer bytes | More bytes | New site |")
    print("|---|---|---|---|---|---|---|")
    for (model, cond), by_task in sorted(runs.items()):
        c = Counter()
        for t, rs in by_task.items():
            if track.get(t) != 2 or t not in base:
                continue
            for r in rs:
                if r["F"] != 1 or r["R"] != 0:
                    continue
                al = (scored(r).get("result") or {}).get("alloc") or {}
                b0, b1 = base[t].get("bytes", 0), al.get("bytes", 0)
                if site_fns(al) - site_fns(base[t]):
                    c["new site"] += 1
                elif b1 == b0:
                    c["unchanged"] += 1
                elif b1 < b0:
                    c["fewer bytes"] += 1
                else:
                    c["more bytes"] += 1
        n = sum(c.values())
        if n:
            print(f"| {model} | {cond} | {n} | {c['unchanged']} | {c['fewer bytes']} | {c['more bytes']} | {c['new site']} |")
            out["track2_alloc"][f"{model}|{cond}"] = dict(c)

    print("\n## 5. C2 repair rounds\n")
    for model in models:
        recs = [r for rs in runs.get((model, "C2"), {}).values() for r in rs]
        if not recs:
            continue
        trans = Counter()
        first_clean = Counter()
        for r in recs:
            rounds = r.get("rounds") or []
            for i, (p, q) in enumerate(zip(rounds, rounds[1:])):
                trans[(f"{p['F']}/{p['R']}", f"{q['F']}/{q['R']}")] += 1
            k = next((i for i, x in enumerate(rounds) if x["F"] and x["R"]), None)
            first_clean["never" if k is None else f"round {k}"] += 1
        print(f"{model}: transitions after a malloc-guard report (F/R before -> after):")
        for (p, q), v in sorted(trans.items()):
            print(f"  {p} -> {q}: {v}")
        print(f"  first round with F=1, R=1: " + ", ".join(f"{k} {v}" for k, v in sorted(first_clean.items())))
        last_worse = sum(1 for r in recs if r["F"] * r["R"] == 0 and
                         any(x["F"] * x["R"] for x in (r.get("rounds") or [])))
        broke = sum(1 for r in recs if r["F"] == 0 and any(x["F"] for x in (r.get("rounds") or [])))
        print(f"  runs whose last patch is worse than an earlier clean one: {last_worse}; "
              f"runs that passed the tests in some round but not in the last: {broke}\n")
        out["c2_rounds"][model] = {"transitions": {f"{p}->{q}": v for (p, q), v in trans.items()},
                                   "first_clean": dict(first_clean), "last_worse": last_worse, "broke": broke}

    print("## 6. RT-safe resolved rate per task (mean over samples)\n")
    for model in models:
        conds = [c for c in ("C0", "C1", "C2", "C2-best", "C3") if (model, c) in rt_by]
        print(f"{model}\n")
        print("| Task | Track | " + " | ".join(conds) + " |")
        print("|---|---|" + "---|" * len(conds))
        for t in sorted(track):
            row = [rt_by[(model, c)].get(t) for c in conds]
            print(f"| {t} | {track[t]} | " + " | ".join("" if v is None else f"{v:.2f}" for v in row) + " |")
            out["per_task"].setdefault(model, {})[t] = dict(zip(conds, row))
        print()

    print("## 7. Cost per run (mean)\n")
    print("| Model | Cond. | Steps | Seconds | Prompt tokens | Completion tokens |")
    print("|---|---|---|---|---|---|")
    for (model, cond), by_task in sorted(runs.items()):
        if cond == "C2-best":
            continue
        recs = [r for rs in by_task.values() for r in rs]
        n = len(recs)
        st = sum(r.get("steps", 0) for r in recs) / n
        sec = sum(r.get("seconds", 0) for r in recs) / n
        pt = sum((r.get("usage") or {}).get("prompt_tokens", 0) for r in recs) / n
        ct = sum((r.get("usage") or {}).get("completion_tokens", 0) for r in recs) / n
        print(f"| {model} | {cond} | {st:.0f} | {sec:.0f} | {pt:,.0f} | {ct:,.0f} |")
        out["cost"][f"{model}|{cond}"] = {"steps": st, "seconds": sec, "prompt_tokens": pt, "completion_tokens": ct}

    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1))
    if a.latex:
        hw = {p.parent.name for p in tasks.glob("*/task.json")
              if "hand-written" in json.loads(p.read_text()).get("source", "")}
        Path(a.latex).write_text(latex_table(runs, track, hw))


MODELS = [("gemma-4-31B-it-qat-w4a16-ct", "Gemma~4 31B"), ("gemma-4-12B-it-qat-w4a16-ct", "Gemma~4 12B")]
CONDS = [("C0", "C0 zero-shot"), ("C1", "C1 RT prompt"), ("C2", "C2 \\mg report"),
         ("C2-best", "\\quad best round$^\\dagger$"), ("C3", "C3 call graph")]


def latex_table(runs, track, hw=frozenset()):
    """Main results table: Track 1 func, RT, gap; Track 2 RT, regression; 95% CIs over tasks.

    Every number in a column is padded with \\phantom digits to the widest one in that column, so
    values and interval brackets line up vertically (the text font has tabular figures)."""
    t1 = sorted(t for t in track if track[t] == 1)
    t2 = sorted(t for t in track if track[t] == 2)
    body = []  # (label or None for a model header, name, [cells]); a cell is (m, lo, hi) or None
    for model, name in MODELS:
        body.append((None, name, None))
        for cond, label in CONDS:
            by_task = runs.get((model, cond), {})
            f_t = per_task(by_task, lambda r: r["F"])
            fr_t = per_task(by_task, lambda r: r["F"] * r["R"])
            a = [t for t in t1 if t in by_task]
            b = [t for t in t2 if t in by_task]
            cols = [[f_t[t] for t in a], [fr_t[t] for t in a], [f_t[t] - fr_t[t] for t in a],
                    [fr_t[t] for t in b], [1 - f_t[t] for t in b],
                    [f_t[t] - fr_t[t] for t in b],  # Track 2: tests pass, allocation remains
                    [f_t[t] for t in a if t not in hw], [f_t[t] for t in a if t in hw]]  # Track 1 split
            cells = []
            for v in cols:
                m, lo, hi = boot_ci(v)
                cells.append(None if m is None else tuple(round(100 * x) for x in (m, lo, hi)))
            body.append((label, None, cells))
    width = [[1, 1, 1] for _ in range(8)]  # digits of mean, lower, upper per column
    for label, _, cells in body:
        for c, cell in enumerate(cells or []):
            if cell:
                width[c] = [max(w, len(str(x))) for w, x in zip(width[c], cell)]

    def pad(x, w):
        d = len(str(x))
        return ("\\phantom{" + "0" * (w - d) + "}" if w > d else "") + str(x)

    def fmt(c, cell):
        if cell is None:
            return "\\tbd"
        m, lo, hi = cell
        wm, wl, wh = width[c]
        return f"{pad(m, wm)}\\ci{{{pad(lo, wl)}}}{{{pad(hi, wh)}}}"

    # Track 1: when no attempt in any condition has a hidden violation, Resolved and RT-safe are the same
    # number and the hidden-violation column is all zeros; show one column then (the caption says HV = 0).
    no_hv = all(cells[2] is not None and cells[2][0] == 0 and cells[2][2] == 0
                for label, _, cells in body if label is not None)
    # Track 2 columns: remediated, still allocating, regressed (they add up to 100 per row)
    keep = [6, 7, 3, 5, 4] if no_hv else [0, 1, 2, 3, 5, 4]
    n1 = 2 if no_hv else 3
    n_up, n_hw = len([t for t in t1 if t not in hw]), len([t for t in t1 if t in hw])
    head1 = f"Upstream ({n_up}) $\\uparrow$ & Hand-written ({n_hw}) $\\uparrow$" if no_hv else \
        "Resolved $\\uparrow$ & RT-safe $\\uparrow$ & Hidden viol. $\\downarrow$"
    lines = [
        "% generated by scripts/analyze_runs.py --latex; do not edit by hand"
        + ("; Track 1 hidden-violation rate is 0 in every condition, one column" if no_hv else ""),
        "\\begin{tabular}{@{}l" + "c" * (n1 + 3) + "@{}}",
        "  \\toprule",
        f"  & \\multicolumn{{{n1}}}{{c}}{{Bug fixes: resolved}}"
        f" & \\multicolumn{{3}}{{c}}{{Allocation removals ({len(t2)} tasks)}} \\\\",
        f"  \\cmidrule(lr){{2-{1 + n1}}}\\cmidrule(l){{{2 + n1}-{4 + n1}}}",
        f"  Condition & {head1} & Remediated $\\uparrow$ & Still alloc. $\\downarrow$ & Regressed $\\downarrow$ \\\\",
    ]
    for label, name, cells in body:
        if label is None:
            lines += ["  \\midrule", f"  \\multicolumn{{{n1 + 4}}}{{@{{}}l}}{{\\textit{{{name}}}}} \\\\"]
        else:
            lines.append(f"  {label} & " + " & ".join(fmt(c, cells[c]) for c in keep) + " \\\\")
    lines += ["  \\bottomrule", "\\end{tabular}", ""]
    return "\n".join(lines)


if __name__ == "__main__":
    main()
