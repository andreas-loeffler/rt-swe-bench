#!/usr/bin/env python3
"""Summarize the LV2 census (results/lv2/lv2_census.jsonl).

    python3 census/lv2/census_table.py [results/lv2/lv2_census.jsonl]

A plugin allocates if malloc-guard recorded any allocation in its 400 guarded
run() calls (after 16 unguarded warm-up runs). Pattern, as in the controller
census: steady = in >= 90% of runs, once = only in the first 3 guarded runs,
otherwise sporadic (periodic or event-driven, e.g. on a MIDI note).
"""
import collections
import json
import sys


def pattern(r: dict) -> str:
    if r["allocating_runs"] >= 0.9 * r["runs"]:
        return "steady"
    if r["last_run"] < 3:
        return "once"
    return "sporadic"


def main() -> None:
    path = sys.argv[1] if len(sys.argv) > 1 else "results/lv2/lv2_census.jsonl"
    rows = [json.loads(l) for l in open(path)]
    ok = [r for r in rows if r.get("status") == "ok"]
    status = collections.Counter(r.get("status", "?").split(":")[0] for r in rows)
    hard = [r for r in ok if r.get("hard_rt")]
    alloc = [r for r in ok if r["allocs"] > 0]
    hard_alloc = [r for r in hard if r["allocs"] > 0]
    print(f"plugins: {len(rows)}  measured: {len(ok)}  status: {dict(status)}")
    print(f"declare lv2:hardRTCapable: {len(hard)} of {len(ok)} measured")
    print(f"allocate in run(): {len(alloc)} of {len(ok)}; of the hardRTCapable ones: {len(hard_alloc)} of {len(hard)}")
    print("patterns (hardRTCapable, allocating):", dict(collections.Counter(pattern(r) for r in hard_alloc)))
    print("hardRTCapable allocating plugins taking MIDI:", sum(r.get("midi", False) for r in hard_alloc))

    print("\n| Package | Measured | hardRT | hardRT allocating | steady | sporadic | once |")
    print("|---|---|---|---|---|---|---|")
    by = collections.defaultdict(list)
    for r in ok:
        by[r.get("package") or "?"].append(r)
    for pkg, rs in sorted(by.items(), key=lambda kv: -len(kv[1])):
        h = [r for r in rs if r.get("hard_rt")]
        ha = [r for r in h if r["allocs"] > 0]
        c = collections.Counter(pattern(r) for r in ha)
        print(f"| {pkg} | {len(rs)} | {len(h)} | {len(ha)} | {c['steady']} | {c['sporadic']} | {c['once']} |")

    print("\nTop allocating hardRTCapable plugins (bytes per guarded run):")
    for r in sorted(hard_alloc, key=lambda r: -r["bytes"] / r["runs"])[:15]:
        site = max(r["sites"], key=lambda s: s["allocs"])["site"] if r["sites"] else "?"
        print(f"  {r['bytes'] / r['runs']:9.1f} B/run  {r['allocs']:6d} allocs in {r['allocating_runs']:3d} runs  "
              f"{pattern(r):8s} {r.get('package', '?'):24s} {r['name'][:40]:40s} {site[:70]}")

    skips = collections.Counter(r["status"] for r in rows if r.get("status", "").startswith("skip"))
    if skips:
        print("\nSkipped (unsupported required features or ports):")
        for s, n in skips.most_common(8):
            print(f"  {n:4d}  {s}")


if __name__ == "__main__":
    main()
