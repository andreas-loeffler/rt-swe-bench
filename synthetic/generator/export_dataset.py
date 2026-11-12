#!/usr/bin/env python3
"""Export the synthetic dataset from a generation run (synthetic/generator/generate.py output) into the repository.

For every accepted candidate, the task files: spec.md, reference.hpp, reference_2.hpp (the second model's
independent reference), stub.hpp (start of the implementation task), test.cpp and the allocating variants
that passed gate 3 (variants/<pattern>.hpp, one remediation task each), plus provenance.json without the
raw model answers (generator, cross-checker, prompt version, sampling, gate results, repair rounds).
candidates.jsonl lists all candidates, accepted or not, with the gate that rejected them, so the yield
table (synthetic/generator/generate.py report) can be recomputed from the export alone.

    python3 synthetic/generator/export_dataset.py synthetic/generator/out/tier1_v3/candidates synthetic/dataset
"""
import json
import shutil
import sys
from pathlib import Path

KEEP = ["spec.md", "reference.hpp", "reference_2.hpp", "stub.hpp", "test.cpp"]
PROV = ["id", "idea", "generator", "cross_checker", "prompt_version", "sampling", "gates", "repairs", "accepted",
        "rejected", "round", "test_include_normalized"]


def main(src: Path, dst: Path) -> None:
    dst.mkdir(parents=True, exist_ok=True)
    rows, n_acc, n_var = [], 0, 0
    for cand in sorted(p for p in src.iterdir() if (p / "provenance.json").exists()):
        prov = json.loads((cand / "provenance.json").read_text())
        idea = prov.get("idea", {})
        rows.append({"id": prov["id"], "domain": idea.get("domain"), "level": idea.get("level"),
                     "generator": idea.get("generator"), "accepted": bool(prov.get("accepted")),
                     "rejected": prov.get("rejected"), "repairs": len(prov.get("repairs") or []),
                     "variants": [v.removesuffix(".hpp") for v in prov.get("gates", {}).get("G3", {}).get("kept", [])]
                     if prov.get("accepted") else []})
        if not prov.get("accepted"):
            continue
        out = dst / "kernels" / prov["id"]
        (out / "variants").mkdir(parents=True, exist_ok=True)
        for f in KEEP:
            if (cand / f).exists():
                shutil.copy(cand / f, out / f)
        kept = [v.removesuffix(".hpp") for v in prov["gates"]["G3"]["kept"]]  # file names in the gate log
        for v in kept:
            shutil.copy(cand / "variants" / f"{v}.hpp", out / "variants" / f"{v}.hpp")
        (out / "provenance.json").write_text(json.dumps({k: prov[k] for k in PROV if k in prov}, indent=1) + "\n")
        n_acc, n_var = n_acc + 1, n_var + len(kept)
    with (dst / "candidates.jsonl").open("w") as f:
        for r in rows:
            f.write(json.dumps(r) + "\n")
    print(f"{len(rows)} candidates, {n_acc} kernels exported, {n_acc} implementation and {n_var} remediation tasks")


if __name__ == "__main__":
    main(Path(sys.argv[1]), Path(sys.argv[2]))
