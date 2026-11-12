#!/usr/bin/env python3
"""Re-score, with test-file edits dropped (scripts/evaluate.sh since 2026-11-08), every scored patch that
touches a test file and was scored before that change. Output: results/rescore/test_edits_dropped.jsonl,
passed to the analysis scripts after the per-model files so that its verdicts take precedence."""
import json, re, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from rescore_runs import ROOT, jobs, score  # noqa: E402

out = ROOT / "results/rescore/test_edits_dropped.jsonl"
done = {json.loads(l)["id"] for l in out.read_text().splitlines()} if out.exists() else set()
todo = [j for j in jobs(ROOT / "results/runs") if j["id"] not in done
        and re.search(r"(?m)^diff --git a/\S*(^|/)tests?/", j["patch"])]
print(len(todo), "patches with test-file edits", flush=True)
with ThreadPoolExecutor(2) as ex:
    for res in ex.map(lambda j: score(j, ROOT / "results/raw/rescore_testedits"), todo):
        with out.open("a") as fh:
            fh.write(json.dumps(res) + "\n")
        print(res["id"], res["recorded"]["F"], res["recorded"]["R"], "->", res["rescored"].get("F"), res["rescored"].get("R"), flush=True)
