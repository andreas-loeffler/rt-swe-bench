#!/usr/bin/env python3
"""Blind human audit of generated synthetic tasks: a stratified sample of accepted kernels as one local HTML page.

The sample is drawn per (generator, level) stratum with a fixed seed, so it is reproducible. The page hides
which model generated a kernel (blind audit); the key is written to a separate file. The auditor answers five
yes/no questions per kernel plus notes; answers stay in the browser (localStorage) and are exported as JSON
with one button. No network, no server: open the file in a browser.

    python3 synthetic/generator/audit.py --out synthetic/generator/out/tier1_v3 [--fraction 0.1] [--seed 0]
    -> synthetic/generator/out/tier1_v3/audit/audit.html, audit_key.json (generator per item; do not open before the audit)
    python3 synthetic/generator/audit.py --out synthetic/generator/out/tier1_v3 --summary audit_answers.json   # after the audit
"""
from __future__ import annotations

import argparse
import difflib
import html
import json
import math
import random
from collections import Counter, defaultdict
from pathlib import Path

QUESTIONS = [
    ("spec", "The spec is clear and complete: an engineer could implement the class from spec.md and the stub alone."),
    ("reference", "The reference is correct, and its real-time methods do not allocate (judged by reading)."),
    ("test", "The test checks the behavior the spec describes, not details of one implementation."),
    ("variant", "Each variant is a plausible developer mistake that differs from the reference only by allocating."),
    ("level", "The difficulty level fits the code (easy: one class, one method, one buffer; medium: shared state "
              "across several methods; hard: helpers several calls deep, modes)."),
]


def sample(cands: Path, fraction: float, seed: int) -> list[dict]:
    strata = defaultdict(list)
    for p in sorted(cands.glob("*/provenance.json")):
        prov = json.loads(p.read_text())
        if prov.get("accepted"):
            strata[(prov["generator"], prov["idea"].get("level", "?"))].append(prov)
    rng = random.Random(seed)
    picked = []
    for key in sorted(strata):
        items = strata[key]
        k = min(len(items), max(1, math.ceil(fraction * len(items))))
        picked += rng.sample(items, k)
    rng.shuffle(picked)  # mixes generators and levels on the page
    return picked


def block(title: str, text: str, lang: str = "") -> str:
    return (f"<details{' open' if lang != 'test' else ''}><summary>{html.escape(title)}</summary>"
            f"<pre>{html.escape(text)}</pre></details>")


def diff_block(title: str, a: str, b: str) -> str:
    rows = []
    for line in difflib.unified_diff(a.splitlines(), b.splitlines(), "reference.hpp", title, lineterm="", n=2):
        cls = "add" if line.startswith("+") and not line.startswith("+++") else \
              "del" if line.startswith("-") and not line.startswith("---") else ""
        rows.append(f'<span class="{cls}">{html.escape(line)}</span>')
    return f"<details open><summary>variant {html.escape(title)} (diff to the reference)</summary><pre>" + \
        "\n".join(rows) + "</pre></details>"


def page(items: list[dict], cands: Path) -> str:
    out = []
    for n, prov in enumerate(items, 1):
        d = cands / prov["id"]
        ref = (d / "reference.hpp").read_text()
        variants = "".join(diff_block(v, ref, (d / "variants" / v).read_text())
                           for v in prov["gates"]["G3"].get("kept", []))
        qs = "".join(
            f'<div class="q"><span>{html.escape(t)}</span><label><input type="radio" name="{n}_{k}" value="yes"> yes</label>'
            f'<label><input type="radio" name="{n}_{k}" value="no"> no</label>'
            f'<label><input type="radio" name="{n}_{k}" value="unsure"> unsure</label></div>' for k, t in QUESTIONS)
        out.append(f'''<section id="item{n}" data-item="{n}">
<h2>{n}. {html.escape(prov["idea"]["name"])} <small>{html.escape(prov["idea"]["domain"])}, level
{html.escape(prov["idea"].get("level", "?"))}</small></h2>
{block("spec.md", (d / "spec.md").read_text())}
{block("reference.hpp", ref)}
{variants}
{block("test.cpp", (d / "test.cpp").read_text(), "test")}
{qs}
<textarea name="{n}_notes" placeholder="notes (what is wrong, if anything)"></textarea>
</section>''')
    return TEMPLATE.replace("{{N}}", str(len(items))).replace("{{ITEMS}}", "\n".join(out))


TEMPLATE = r'''<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1"><title>Synthetic task audit</title>
<style>
:root { --bg:#fff; --fg:#1d1d1f; --muted:#6e6e73; --line:#d9d9d9; --code:#f6f6f6; --add:#e6f4ea; --del:#fde8e1; --accent:#3182bd; }
@media (prefers-color-scheme: dark) { :root { --bg:#161617; --fg:#ececec; --muted:#9a9aa0; --line:#333;
  --code:#1f1f21; --add:#16351f; --del:#3d1f17; --accent:#6baed6; } }
body { background:var(--bg); color:var(--fg); font:15px/1.5 system-ui, sans-serif; margin:0 auto; max-width:980px; padding:16px; }
header { position:sticky; top:0; background:var(--bg); border-bottom:1px solid var(--line); padding:8px 0; z-index:1; }
section { border-bottom:1px solid var(--line); padding:12px 0 20px; }
h2 { font-size:18px; margin:8px 0; } h2 small { color:var(--muted); font-weight:400; }
pre { background:var(--code); padding:10px; overflow-x:auto; font-size:12.5px; line-height:1.4; border-radius:4px; }
summary { cursor:pointer; color:var(--accent); margin:6px 0; }
.add { background:var(--add); display:block; } .del { background:var(--del); display:block; }
.q { display:flex; flex-wrap:wrap; gap:10px; align-items:baseline; margin:6px 0; }
.q span { flex:1 1 420px; } label { white-space:nowrap; }
textarea { width:100%; min-height:48px; margin-top:6px; background:var(--code); color:var(--fg); border:1px solid var(--line); }
button { font:inherit; padding:4px 12px; }
</style></head><body>
<header><b>Synthetic task audit</b>: {{N}} kernels, generator hidden. <span id="done"></span>
<button onclick="exportJSON()">Export answers (JSON)</button></header>
<p>Answer the five questions for each kernel. Answers are kept in this browser; export them when done and give the
JSON file to the analysis (<code>synthetic/generator/audit.py --summary</code>). The test is collapsed; open it when needed.</p>
{{ITEMS}}
<script>
const KEY = "rtswe-audit-v1";
function load() { try { return JSON.parse(localStorage.getItem(KEY) || "{}"); } catch (e) { return {}; } }
function save(a) { try { localStorage.setItem(KEY, JSON.stringify(a)); } catch (e) {} }
const ans = load();
document.querySelectorAll("input[type=radio], textarea").forEach(el => {
  if (el.type === "radio") { el.checked = ans[el.name] === el.value; }
  else { el.value = ans[el.name] || ""; }
  el.addEventListener("change", () => { ans[el.name] = el.value; save(ans); progress(); });
  el.addEventListener("input", () => { if (el.tagName === "TEXTAREA") { ans[el.name] = el.value; save(ans); } });
});
function progress() {
  const n = document.querySelectorAll("section").length, q = 5;
  const done = [...document.querySelectorAll("section")].filter(s =>
    [...s.querySelectorAll("input[type=radio]")].filter(i => i.checked).length === q).length;
  document.getElementById("done").textContent = `${done} of ${n} complete`;
}
function exportJSON() {
  const blob = new Blob([JSON.stringify(ans, null, 1)], {type: "application/json"});
  const a = document.createElement("a"); a.href = URL.createObjectURL(blob); a.download = "audit_answers.json"; a.click();
}
progress();
</script></body></html>'''


def summary(answers: dict, key: dict) -> None:
    per = defaultdict(Counter)
    for name, val in answers.items():
        item, q = name.split("_", 1)
        if q == "notes" or item not in key:
            continue
        for group in ("all", key[item]["generator"], key[item]["level"]):
            per[(group, q)][val] += 1
    print("| Group | Question | yes | no | unsure |\n|---|---|---|---|---|")
    for (g, q), c in sorted(per.items()):
        print(f"| {g} | {q} | {c['yes']} | {c['no']} | {c['unsure']} |")
    notes = {k: v for k, v in answers.items() if k.endswith("_notes") and v.strip()}
    for k, v in sorted(notes.items(), key=lambda kv: int(kv[0].split("_")[0])):
        i = k.split("_")[0]
        print(f"- item {i} ({key.get(i, {}).get('id', '?')}): {v.strip()}")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True, help="output directory of a synthetic/generator/generate.py run")
    ap.add_argument("--fraction", type=float, default=0.1)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--summary", help="exported answers (JSON): print the results per generator and level")
    a = ap.parse_args()
    out = Path(a.out)
    adir = out / "audit"
    if a.summary:
        summary(json.loads(Path(a.summary).read_text()), json.loads((adir / "audit_key.json").read_text()))
        return
    items = sample(out / "candidates", a.fraction, a.seed)
    adir.mkdir(parents=True, exist_ok=True)
    (adir / "audit.html").write_text(page(items, out / "candidates"))
    gen_tag = lambda m: "31B" if "31B" in m else "12B" if "12B" in m else m  # noqa: E731
    (adir / "audit_key.json").write_text(json.dumps(
        {str(n): {"id": p["id"], "generator": gen_tag(p["generator"]), "level": p["idea"].get("level", "?")}
         for n, p in enumerate(items, 1)}, indent=1))
    strata = Counter((gen_tag(p["generator"]), p["idea"].get("level", "?")) for p in items)
    print(f"{len(items)} kernels sampled ({', '.join(f'{g} {lv}: {n}' for (g, lv), n in sorted(strata.items()))})")
    print(f"page: {adir / 'audit.html'}  key (do not open before the audit): {adir / 'audit_key.json'}")


if __name__ == "__main__":
    main()
