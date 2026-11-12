#!/usr/bin/env python3
"""Track 2 ablation of the feedback conditions (Gemma 4 31B): what makes the malloc-guard report help?

Conditions on the 10 Track 2 tasks: C1 (RT prompt), C2 (full report: sites and backtrace, up to 3 repair
rounds), C2B (repair rounds that say only how many bytes update() still allocates), C2N (3 extra rounds
with a generic review request after every submission, no verdict), C4 (C1 plus the unpatched code's
report, i.e. the location, before the first action), and C0/C3 from the main matrix.

Unit of analysis is the task (mean over its samples). CIs: percentile bootstrap over tasks. Paired
comparisons: per-task differences, exact sign-flip test. Primary comparison (fixed before the ablation
run): C2 vs C1 on the remediation rate. Secondary comparisons are Holm-corrected as one family.
Also reported without the three pilot tasks (rt-001..003), on which the harness was debugged.

    python3 scripts/analyze_ablation.py [--rescore results/rescore/<model>.jsonl] [--latex t.tex] [--fig f.tex]
"""
import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from analyze_runs import boot_ci, load, load_rescore, per_task, sign_flip_p  # noqa: E402

MODEL = "gemma-4-31B-it-qat-w4a16-ct"
CONDS = [("C0", "C0 zero-shot"), ("C1", "C1 RT prompt"), ("C3", "C3 call graph"), ("C4", "C4 location up front"),
         ("C1T", "C1 + thinking"), ("C4T", "C4 + thinking"),
         ("C2N", "C2N review rounds, no verdict"), ("C2B", "C2B verdict, no location"),
         ("C2", "C2 verdict and location"), ("C2-best", "C2 best round")]
PRIMARY = ("C2", "C1")
SECONDARY = [("C2B", "C1"), ("C2N", "C1"), ("C4", "C1"), ("C2", "C2B"), ("C2B", "C2N"), ("C2", "C2N"),
             ("C2B", "C4")]
PILOT = {"rt-001", "rt-002", "rt-003"}
# Thinking on: C1 and C4 with Gemma 4's thinking mode, runs in <model>-thinking.
# Added after the main ablation, so its comparisons form their own Holm family and leave the others unchanged.
THINKING = [("C1T", "C1"), ("C4T", "C4"), ("C2B", "C1T")]


def strat_perm_p(runs_x, runs_y, n=20000, seed=20261108):
    """Two-sided permutation p-value, condition labels shuffled within each task (runs are exchangeable
    within a task under the null); statistic: mean over tasks of the difference in remediation.
    Uses all runs while keeping the task as the stratum, so it has more power than the sign-flip test
    on per-task means when tasks have several samples."""
    import random
    rng = random.Random(seed)
    strata = [(list(runs_x[t]), list(runs_y[t])) for t in sorted(set(runs_x) & set(runs_y))]
    def stat(st):
        return sum(sum(x) / len(x) - sum(y) / len(y) for x, y in st) / len(st)
    obs = abs(stat(strata))
    hits = 0
    for _ in range(n):
        st = []
        for x, y in strata:
            pool = x + y
            rng.shuffle(pool)
            st.append((pool[:len(x)], pool[len(x):]))
        hits += abs(stat(st)) >= obs - 1e-12
    return (hits + 1) / (n + 1)


def holm(ps):
    order = sorted(range(len(ps)), key=lambda i: ps[i])
    adj, run = [0.0] * len(ps), 0.0
    for k, i in enumerate(order):
        run = max(run, min(1.0, (len(ps) - k) * ps[i]))
        adj[i] = run
    return adj


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--runs", default="results/runs")
    ap.add_argument("--tasks", default="tasks")
    ap.add_argument("--rescore", nargs="*", default=[])
    ap.add_argument("--latex")
    ap.add_argument("--fig")
    ap.add_argument("--json")
    a = ap.parse_args()

    track = {p.parent.name: json.loads(p.read_text())["track"] for p in Path(a.tasks).glob("*/task.json")}
    t2 = sorted(t for t, k in track.items() if k == 2)
    runs = load(Path(a.runs), load_rescore(a.rescore))
    for c in ("C1", "C4"):  # thinking runs as conditions C1T, C4T of the same model
        if (MODEL + "-thinking", c) in runs:
            runs[(MODEL, c + "T")] = runs[(MODEL + "-thinking", c)]
    out = {"rates": {}, "paired": [], "nopilot": {}}

    def rates(cond, tasks):
        by = {t: rs for t, rs in runs[(MODEL, cond)].items() if t in tasks}
        rem = per_task(by, lambda r: r["F"] * r["R"])
        still = per_task(by, lambda r: r["F"] * (1 - r["R"]))
        reg = per_task(by, lambda r: 1 - r["F"])
        n = sum(len(v) for v in by.values())
        return rem, still, reg, n

    for subset, tasks in (("all", set(t2)), ("nopilot", set(t2) - PILOT)):
        print(f"\n## Track 2 outcome rates, 31B ({subset}: {len(tasks)} tasks), % with 95% bootstrap CI over tasks\n")
        print("| Condition | Runs | Remediated | Still allocates | Regressed |")
        print("|---|---|---|---|---|")
        for c, label in CONDS:
            if (MODEL, c) not in runs:
                continue
            rem, still, reg, n = rates(c, tasks)
            row = [boot_ci(list(x.values())) for x in (rem, still, reg)]
            out["rates" if subset == "all" else "nopilot"][c] = {"runs": n, "rem": row[0], "still": row[1],
                                                                  "reg": row[2]}
            print(f"| {label} | {n} | " + " | ".join(f"{100 * m:.0f} [{100 * lo:.0f}, {100 * hi:.0f}]"
                                                    for m, lo, hi in row) + " |")

        print(f"\n## Paired differences in remediation ({subset}), points; primary: C2 vs C1, "
              "secondary Holm-corrected\n")
        print("| Comparison | Tasks | Diff | 95% CI | better / worse | p sign-flip | p strat. perm. | p perm. (Holm) |")
        print("|---|---|---|---|---|---|---|---|")
        rows = []
        thinking = [p for p in THINKING if (MODEL, p[0]) in runs and (MODEL, p[1]) in runs]
        for x, y in [PRIMARY] + SECONDARY + thinking:
            rx, ry = rates(x, tasks)[0], rates(y, tasks)[0]
            ts = sorted(set(rx) & set(ry))
            d = [rx[t] - ry[t] for t in ts]
            m, lo, hi = boot_ci(d)
            ox = {t: [r["F"] * r["R"] for r in rs] for t, rs in runs[(MODEL, x)].items() if t in tasks}
            oy = {t: [r["F"] * r["R"] for r in rs] for t, rs in runs[(MODEL, y)].items() if t in tasks}
            rows.append({"x": x, "y": y, "tasks": len(ts), "diff": [m, lo, hi], "p_sf": sign_flip_p(d),
                         "p": strat_perm_p(ox, oy),
                         "better": sum(v > 0 for v in d), "worse": sum(v < 0 for v in d), "subset": subset})
        n2 = 1 + len(SECONDARY)
        rows[0]["p_holm"] = rows[0]["p"]
        for r, p in zip(rows[1:n2], holm([r["p"] for r in rows[1:n2]])):
            r["p_holm"] = p
        for r, p in zip(rows[n2:], holm([r["p"] for r in rows[n2:]])):  # thinking: own family
            r["p_holm"], r["family"] = p, "thinking"
        for r in rows:
            print(f"| {r['x']} vs {r['y']} | {r['tasks']} | {100 * r['diff'][0]:+.0f} | "
                  f"[{100 * r['diff'][1]:+.0f}, {100 * r['diff'][2]:+.0f}] | {r['better']} / {r['worse']} | "
                  f"{r['p_sf']:.3f} | {r['p']:.4f} | {r['p_holm']:.4f} |")
        out["paired"] += rows

    # per task remediation, for the text
    print("\n## Remediation per task (%), 31B\n")
    shown = [c for c, _ in CONDS if (MODEL, c) in runs and c != "C2-best"]
    print("| Task | " + " | ".join(shown) + " |")
    print("|---|" + "---|" * len(shown))
    for t in t2:
        cells = []
        for c in shown:
            rs = runs[(MODEL, c)].get(t, [])
            cells.append(f"{100 * sum(r['F'] * r['R'] for r in rs) / len(rs):.0f} (n={len(rs)})" if rs else "-")
        print(f"| {t} | " + " | ".join(cells) + " |")

    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1))
    r = out["rates"]
    if a.latex:
        def ci(x):
            m, lo, hi = x
            pad = lambda v: f"\\phantom{{0}}{v:.0f}" if v < 9.5 else f"{v:.0f}"  # noqa: E731
            return f"{100 * m:.0f}\\ci{{{pad(100 * lo)}}}{{{100 * hi:.0f}}}"
        lines = ["% generated by scripts/analyze_ablation.py --latex; do not edit by hand",
                 "\\begin{tabular}{@{}lcccc@{}}", "  \\toprule",
                 "  Condition & Runs & Remediated $\\uparrow$ & Still alloc. $\\downarrow$ & Regressed $\\downarrow$ \\\\",
                 "  \\midrule"]
        for c, label in CONDS:
            if c in r:
                lines.append(f"  {label} & {r[c]['runs']} & {ci(r[c]['rem'])} & {ci(r[c]['still'])} & "
                             f"{ci(r[c]['reg'])} \\\\")
        lines += ["  \\bottomrule", "\\end{tabular}", ""]
        Path(a.latex).write_text("\n".join(lines))
    if a.fig:
        write_fig(Path(a.fig), r)


# Figure 4: a dot plot (Cleveland), one panel per outcome, so every estimate sits on a common scale and
# carries its own 95% CI (a stacked bar puts only its first segment on a common scale, and a whisker on a
# stacked bar is easy to read as part of the next segment). Thinking rows sit under their base condition,
# with open markers; the dashed line in each panel is C1, the reference of the paired comparisons.
FIG_ROWS = [("C0", "C0 zero-shot"), ("C1", "C1 RT prompt"), ("C1T", "+ thinking"), ("C3", "C3 call graph"),
            ("C4", "C4 location up front"), ("C4T", "+ thinking"), ("C2N", "C2N review rounds"),
            ("C2B", "C2B verdict, no location"), ("C2", "C2 verdict and location")]
FIG_PANELS = [("rem", "Remediated", "rtgreen"), ("still", "Tests pass, still allocates", "rtorange"),
              ("reg", "Regressed", "rtgray")]


def write_fig(path: Path, r: dict) -> None:
    rows = [(c, lab) for c, lab in FIG_ROWS if c in r]
    verdict = [c for c, _ in rows if c in ("C2B", "C2")]
    ys, y = {}, 0.0
    for c, _ in rows:
        y += 1 + (0.6 if c == (verdict[0] if verdict else None) else 0)
        ys[c] = y
    gap = ys[verdict[0]] - 0.8 if verdict else None
    ymin, ymax = 0.4, y + 0.6
    unit = 0.40  # cm per row
    geom = (f"scale only axis, width=2.35cm, height={unit * (ymax - ymin):.2f}cm, xmin=0, xmax=100, "
            f"ymin={ymin:g}, ymax={ymax:g}, y dir=reverse, xtick={{0,50,100}}, minor xtick={{25,75}}, "
            "xmajorgrids, xminorgrids, grid style={black!8}, minor grid style={black!8}, axis x line*=bottom, "
            "axis y line*=left, y axis line style={draw=none}, xticklabel style={font=\\scriptsize}, "
            "tick style={black!50}, title style={font=\\footnotesize, yshift=-3pt}, clip=false")
    ticks = ",".join(f"{ys[c]:g}" for c, _ in rows)
    labels = ",".join("{" + lab + r" {\color{black!45}(" + str(r[c]["runs"]) + ")}}" for c, lab in rows)
    out = ["% generated by scripts/analyze_ablation.py --fig; do not edit by hand",
           "% dots: share of attempts per outcome; lines: 95% bootstrap CI over tasks; open dots: thinking on;",
           "% dashed: C1; grey numbers: attempts per condition",
           "\\begin{tikzpicture}"]
    prev = None
    for k, (key, title, col) in enumerate(FIG_PANELS):
        pos = "" if prev is None else f", at={{({prev}.south east)}}, anchor=south west, xshift=0.6cm"
        if k == 1:
            pos += ", xlabel={Share of attempts (\\%)}, xlabel style={font=\\scriptsize, yshift=3pt}"
        yl = (f", ytick={{{ticks}}}, yticklabels={{{labels}}}, yticklabel style={{font=\\footnotesize}}, "
              "ytick style={draw=none}") if k == 0 else ", ytick=\\empty"
        out.append(f"\\begin{{axis}}[name=p{k}, {geom}, title={{{title}}}{pos}{yl}]")
        pct = lambda c, i=0: 100 * r[c][key][i]  # noqa: E731
        if "C1" in r:
            out.append(f"\\draw[black!40, dashed] (axis cs:{pct('C1'):.1f},{ymin:g}) -- (axis cs:{pct('C1'):.1f},{ymax:g});")
        if gap is not None:
            out.append(f"\\draw[black!30] (axis cs:0,{gap:g}) -- (axis cs:100,{gap:g});")
        for c, _ in rows:  # interval as a plain line, the dot on top (open: thinking on)
            out.append(f"\\draw[{col}, line width=0.9pt] (axis cs:{pct(c, 1):.1f},{ys[c]:g}) -- "
                       f"(axis cs:{pct(c, 2):.1f},{ys[c]:g});")
            fill = "white" if c.endswith("T") else col
            out.append(f"\\fill[{fill}, draw={col}, line width=0.9pt] (axis cs:{pct(c):.1f},{ys[c]:g}) circle[radius=1.9pt];")
        for c, _ in rows:  # value next to the interval, on the side with room
            v, lo, hi = pct(c), pct(c, 1), pct(c, 2)
            at, anchor = (hi + 2, "west") if hi <= 86 else (lo - 2, "east")
            out.append(f"\\node[anchor={anchor}, font=\\scriptsize, inner sep=0.5pt, text=black!75] "
                       f"at (axis cs:{at:.1f},{ys[c]:g}) {{{v:.0f}}};")
        if k == len(FIG_PANELS) - 1 and gap is not None:
            plain = [c for c, _ in rows if c not in verdict]
            for a_, b_, text in ((plain[0], plain[-1], "no verdict"), (verdict[0], verdict[-1], "verdict on\\\\own patch")):
                out.append(f"\\draw[decorate, decoration={{brace, amplitude=3pt}}, black!60] (axis cs:104,{ys[a_] - 0.4:g}) -- "
                           f"(axis cs:104,{ys[b_] + 0.4:g}) node[midway, right=4pt, font=\\footnotesize, align=left] {{{text}}};")
        out.append("\\end{axis}")
        prev = f"p{k}"
    out += ["\\end{tikzpicture}", ""]
    path.write_text("\n".join(out))


if __name__ == "__main__":
    main()
