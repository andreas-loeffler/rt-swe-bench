#!/usr/bin/env python3
"""Synthetic task generation: Gemma 4 writes real-time kernels, the dual verifier decides.

Pipeline per kernel idea (README "Next: generating tasks", paper Fig. "Task generation"):
  ideas      each model proposes kernels per domain and difficulty level (synthetic/generator/config.json: easy,
             medium, hard, defined by structure, not by a model's success); a model generates its own
             ideas, so half the kernels come from the 31B and half from the 12B (2x2 design for
             self-bias: both models are later evaluated on both halves)
  kernel     the generator writes spec.md, reference.hpp, stub.hpp, test.cpp
  second     the other model writes a second reference from spec.md and stub.hpp only
  G1         both references pass the test and are allocation-free (F=1, R=1)
  G2         the stub fails the test, and the test kills >= 80% of compiling mutants of the reference
  variants   the generator writes allocating variants, one pattern each (fixed list in config)
  G3         a variant passes the test and allocates (F=1, R=0); the kernel needs at least one
  repair     after a failed G1 or G2, the model that wrote the failing file gets the gate's result
             (compiler errors, failed check, allocations, mutants the test missed) and may fix its
             own files; then all gates run again (synthetic/generator/config.json "repair_rounds", default 2)
No gate looks at whether a model can solve the task. Every candidate keeps its provenance (models,
prompt version, sampling, seeds, raw answers, gate results); rejected ones keep their reason.

    python3 synthetic/generator/generate.py run --out synthetic/generator/out/run [--workers 16] [--domains control,audio_dsp]
    GEN_LLM=offline python3 synthetic/generator/generate.py run --out DIR   # models in-process (synthetic/generator/llm_batch.py), no server
    python3 synthetic/generator/generate.py ideas --out DIR      # only the ideas (before sharded runs)
    python3 synthetic/generator/generate.py run --out DIR --shard 3/8   # one of 8 jobs on a cluster, same ideas file
    python3 synthetic/generator/generate.py selftest            # gates on the hand-written fixtures, no model
    python3 synthetic/generator/generate.py report --out synthetic/generator/out/run
    python3 synthetic/generator/generate.py regate --out synthetic/generator/out/run   # gates again on existing candidates, no model

Scoring: GEN_BACKEND=docker (default, the pinned image, no network) or native (malloc-guard in
$MALLOC_GUARD, default /opt/malloc_guard). Endpoints: GEN_URL_31B / GEN_URL_12B, each one URL or a
comma-separated list (requests rotate over the list, e.g. one vLLM server per GPU).
"""
from __future__ import annotations

import argparse
import concurrent.futures as cf
import hashlib
import itertools
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.request
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mutate  # noqa: E402
import prompts  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
CFG = json.loads((ROOT / "synthetic/generator/config.json").read_text())
IMAGE = os.environ.get("IMAGE", "rt-swe-bench/base:jazzy-d71703b")
BACKEND = os.environ.get("GEN_BACKEND", "docker")
OTHER = {"31b": "12b", "12b": "31b"}
LEVELS = {lv["name"]: lv for lv in CFG["levels"]}
SAMPLING = {**CFG["sampling"], **({"max_tokens": int(os.environ["GEN_MAX_TOKENS"])}
                                  if os.environ.get("GEN_MAX_TOKENS") else {})}
LOCK = threading.Lock()
_RR = {}  # model key -> round-robin iterator over its endpoint URLs


def endpoint(key: str) -> str:
    with LOCK:
        if key not in _RR:
            urls = os.environ.get(f"GEN_URL_{key.upper()}", CFG["models"][key]["base_url"])
            _RR[key] = itertools.cycle([u.strip().rstrip("/") for u in urls.split(",") if u.strip()])
        return next(_RR[key])


def rules(level: str) -> str:
    lv = LEVELS[level]  # RULES contains braces (C++), so substitute instead of str.format
    return (prompts.RULES.replace("{level}", level).replace("{level_about}", lv["about"])
            .replace("{max_lines}", str(lv["max_lines"])))


# ----------------------------------------------------------------------------- model access
def chat(key: str, user: str, seed: int) -> tuple[str, dict]:
    m = CFG["models"][key]
    url = endpoint(key) + "/chat/completions"
    s = SAMPLING
    body = {"model": m["model"], "seed": seed, "temperature": s["temperature"], "top_p": s["top_p"],
            "top_k": s["top_k"], "max_tokens": s["max_tokens"],
            "messages": [{"role": "system", "content": prompts.SYSTEM}, {"role": "user", "content": user}]}
    req = urllib.request.Request(url, data=json.dumps(body).encode(), method="POST", headers={
        "Content-Type": "application/json", "Authorization": f"Bearer {os.environ.get('GEN_API_KEY', 'none')}"})
    for attempt in range(6):
        try:
            with urllib.request.urlopen(req, timeout=900) as r:
                data = json.loads(r.read())
            return data["choices"][0]["message"].get("content") or "", data.get("usage", {})
        except (urllib.error.URLError, TimeoutError, ConnectionError) as e:
            if attempt == 5:
                raise
            print(f"  {key}: endpoint error ({e}), retrying", file=sys.stderr)
            time.sleep(15 * (attempt + 1))
    raise RuntimeError("unreachable")


def files_in(answer: str) -> dict[str, str]:
    out = {}
    for m in re.finditer(r'<file name="([^"]+)">\n?(.*?)</file>', answer, re.S):
        body = m.group(2)
        body = re.sub(r"^\s*```[a-zA-Z+]*\n", "", body)  # tolerate a fence inside the tag
        body = re.sub(r"\n```\s*$", "\n", body)
        out[Path(m.group(1)).name] = body.rstrip() + "\n"
    return out


def normalize_test(src: str) -> tuple[str, bool]:
    """The test must include the implementation under test as "kernel.hpp". Models sometimes
    include their own file name instead (reference.hpp, stub.hpp); then every candidate would be
    scored against that file. Rewrite such includes; returns (source, changed)."""
    new = re.sub(r'#include\s*[<"](?:reference|reference_2|stub|kernel|solution|implementation)\.hpp[>"]',
                 '#include "kernel.hpp"', src)
    return new, new != src


def seed_of(*parts) -> int:
    return int(hashlib.sha256("/".join(map(str, parts)).encode()).hexdigest()[:8], 16)


# ----------------------------------------------------------------------------- scoring
def score(kdir: Path, files: list[Path]) -> dict[str, dict]:
    """Score candidate headers against kdir/test.cpp; returns {file name: result}."""
    if not files:
        return {}
    if BACKEND == "docker":
        cmd = ["docker", "run", "--rm", "--network", "none", "-v", f"{ROOT}:/rtswe:ro",
               "-v", f"{kdir}:/cand:ro", IMAGE, "bash", "/rtswe/synthetic/generator/score_files.sh", "/cand",
               *[f"/cand/{f.relative_to(kdir)}" for f in files]]
    else:
        cmd = ["bash", str(ROOT / "synthetic/generator/score_files.sh"), str(kdir), *map(str, files)]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=3600)
    res = {}
    for line in r.stdout.splitlines():
        try:
            d = json.loads(line)
            res[d.pop("file")] = d
        except (json.JSONDecodeError, KeyError):
            pass
    return res


def gates(kdir: Path, prov: dict) -> bool:
    """G1, G2, G3 on a candidate directory; records results in prov; True if accepted."""
    cfg = CFG
    g = prov["gates"]
    # G1: both references pass both checks
    refs = [kdir / "reference.hpp", kdir / "reference_2.hpp"]
    r = score(kdir, refs + [kdir / "stub.hpp"])
    g["G1"] = {f.name: r.get(f.name) for f in refs}
    bad = [f.name for f in refs if not (r.get(f.name) or {}).get("F") or not (r.get(f.name) or {}).get("R")]
    if bad:
        prov["rejected"] = "G1: " + ", ".join(
            f"{b} F={(r.get(b) or {}).get('F')} R={(r.get(b) or {}).get('R')}" for b in bad)
        return False
    # G2: the stub fails, the test kills most compiling mutants
    stub_f = (r.get("stub.hpp") or {}).get("F")
    mdir = kdir / "mutants"
    mdir.mkdir(exist_ok=True)
    muts = mutate.mutants((kdir / "reference.hpp").read_text(), cfg["mutants_per_kernel"],
                          seed_of(prov["id"], "mutants"))
    names = []
    for i, (desc, src) in enumerate(muts):
        (mdir / f"m{i:02d}.hpp").write_text(f"// mutant: {desc}\n" + src)
        names.append(mdir / f"m{i:02d}.hpp")
    mr = score(kdir, names)
    compiled = [n for n in names if (mr.get(n.name) or {}).get("build")]
    killed = [n for n in compiled if not mr[n.name].get("F")]
    rate = len(killed) / len(compiled) if compiled else 0.0
    g["G2"] = {"stub_F": stub_f, "mutants": len(names), "compiled": len(compiled), "killed": len(killed),
               "kill_rate": round(rate, 3),
               "survivors": [muts[names.index(n)][0] for n in compiled if mr[n.name].get("F")]}
    if stub_f != 0:
        prov["rejected"] = "G2: the stub passes the test"
        return False
    if len(compiled) < cfg["min_compiling_mutants"] or rate < cfg["min_kill_rate"]:
        prov["rejected"] = f"G2: kill rate {rate:.2f} of {len(compiled)} compiling mutants"
        return False
    # G3: each variant passes the test and allocates
    variants = sorted((kdir / "variants").glob("*.hpp")) if (kdir / "variants").exists() else []
    vr = score(kdir, variants)
    g["G3"] = {v.name: vr.get(v.name) for v in variants}
    for v in variants:
        res = vr.get(v.name) or {}
        if not (res.get("F") == 1 and res.get("R") == 0):
            (kdir / "rejected_variants").mkdir(exist_ok=True)
            shutil.move(str(v), kdir / "rejected_variants" / v.name)
    kept = sorted(p.name for p in (kdir / "variants").glob("*.hpp")) if variants else []
    g["G3"]["kept"] = kept
    if not kept:
        prov["rejected"] = "G3: no variant passes the test and allocates"
        return False
    return True


# ----------------------------------------------------------------------------- repair
def problem_of(res: dict | None) -> str:
    """What the gates saw for one scored file, in words for the model."""
    res = res or {}
    if not res.get("build"):
        err = res.get("build_error") or "(no compiler message captured)"
        return f"It does not compile. Compiler errors:\n{err}"
    out = []
    if not res.get("F"):
        mm = res.get("mismatch")
        out.append(f"The test reports a failed check: {mm}" if mm else
                   "The test did not report success (it crashed, timed out, or never printed its result).")
    if not res.get("R"):
        a = res.get("alloc") or {}
        sites = ", ".join(s.get("site", "?") for s in (a.get("sites") or [])[:3])
        out.append(f"The real-time calls allocated heap memory ({a.get('allocs', '?')} allocations"
                   + (f", at {sites}" if sites else "") + ").")
    return "\n".join(out)


def repair_request(kdir: Path, prov: dict) -> dict | None:
    """The repair after failed gates: the model that wrote the failing file gets the result back.
    None if the failure is not one we repair (format, G3)."""
    idea, rnd = prov["idea"], prov["round"]
    gen, cross, lv = idea["generator"], OTHER[idea["generator"]], idea["level"]
    g, rej = prov["gates"], prov["rejected"] or ""
    spec = (kdir / "spec.md").read_text()
    if rej.startswith("G1"):
        r1, r2 = g["G1"].get("reference.hpp"), g["G1"].get("reference_2.hpp")
        if (r1 or {}).get("F") and (r1 or {}).get("R"):
            return {"step": f"repair{rnd}_second", "key": cross, "prompt": prompts.REPAIR_SECOND.format(
                problem=problem_of(r2), spec=spec, reference=(kdir / "reference_2.hpp").read_text(),
                rules=rules(lv), format=prompts.FORMAT)}
        who, problem = gen, problem_of(r1)
    elif rej.startswith("G2: the stub"):
        who = gen
        problem = ("The test also passes with stub.hpp, whose real-time methods only return default values "
                   "(0, false, empty), so the test does not really check the results.")
    elif rej.startswith("G2"):
        who = gen
        surv = g["G2"].get("survivors", [])[:8]
        problem = ("The test still passes when we make small changes to reference.hpp, so it does not check "
                   "the results closely enough. Changes it missed:\n" + "\n".join(f"- {x}" for x in surv)
                   + "\nAdd checks to test.cpp that catch such changes.")
    else:
        return None
    return {"step": f"repair{rnd}_kernel", "key": who, "prompt": prompts.REPAIR_KERNEL.format(
        problem=problem, rules=rules(lv), spec=spec, reference=(kdir / "reference.hpp").read_text(),
        test=(kdir / "test.cpp").read_text(), format=prompts.FORMAT)}


# ----------------------------------------------------------------------------- generation
# Each candidate is a small state machine, saved in provenance.json after every step, so a run can stop
# and continue at any point. States: kernel -> second -> variants -> gates -> (repair -> gates)* -> done.
# The same machine runs per candidate with an HTTP endpoint (run) or for all candidates at once with the
# model loaded in-process, one model at a time (GEN_LLM=offline, see llm_batch.py).

def candidate(idea: dict, out: Path) -> tuple[Path, dict]:
    gen, cross = idea["generator"], OTHER[idea["generator"]]
    kid = f"{idea['domain']}__{idea['level']}__{idea['name']}__{gen}"
    kdir = out / "candidates" / kid
    pfile = kdir / "provenance.json"
    if pfile.exists():
        prov = json.loads(pfile.read_text())
        prov.setdefault("state", "done")  # files written before the state machine are finished
        return kdir, prov
    (kdir / "answers").mkdir(parents=True, exist_ok=True)
    prov = {"id": kid, "idea": idea, "generator": CFG["models"][gen]["model"],
            "cross_checker": CFG["models"][cross]["model"], "prompt_version": CFG["prompt_version"],
            "sampling": SAMPLING, "started": time.time(), "steps": {}, "gates": {}, "rejected": None,
            "state": "kernel", "round": 0}
    save(kdir, prov)
    return kdir, prov


def save(kdir: Path, prov: dict) -> None:
    tmp = kdir / "provenance.json.tmp"
    tmp.write_text(json.dumps(prov, indent=1))
    tmp.replace(kdir / "provenance.json")


def variant_patterns(prov: dict) -> list[dict]:
    pats = CFG["patterns"]
    start = seed_of(prov["id"], "patterns") % len(pats)
    return [pats[(start + j) % len(pats)] for j in range(CFG["variants_per_kernel"])]


def requests_of(kdir: Path, prov: dict) -> list[dict]:
    """The model requests this candidate waits for now: [{"step", "key", "prompt"}]; [] in gates or done."""
    idea, st = prov["idea"], prov["state"]
    gen, cross, lv = idea["generator"], OTHER[idea["generator"]], idea["level"]
    if st == "kernel":
        return [{"step": "kernel", "key": gen, "prompt": prompts.KERNEL.format(
            name=idea["name"], cls=idea["class"], level=lv, domain=idea["domain"], summary=idea["summary"],
            rules=rules(lv), format=prompts.FORMAT)}]
    if st == "second":
        return [{"step": "second_reference", "key": cross, "prompt": prompts.SECOND_REFERENCE.format(
            spec=(kdir / "spec.md").read_text(), stub=(kdir / "stub.hpp").read_text(), rules=rules(lv),
            format=prompts.FORMAT)}]
    if st == "variants":  # independent of each other, so all at once
        spec, ref = (kdir / "spec.md").read_text(), (kdir / "reference.hpp").read_text()
        return [{"step": f"variant_{p['name']}", "key": gen, "prompt": prompts.VARIANT.format(
            pattern=p["name"], about=p["about"], hint=LEVELS[lv]["variant_hint"], spec=spec, reference=ref,
            format=prompts.FORMAT)} for p in variant_patterns(prov) if f"variant_{p['name']}" not in prov["steps"]]
    if st == "repair":
        r = repair_request(kdir, prov)
        return [r] if r else []
    return []


def apply_answer(kdir: Path, prov: dict, req: dict, ans: str, usage: dict, seed: int) -> None:
    """Record one model answer and advance the candidate's state."""
    step, key = req["step"], req["key"]
    (kdir / "answers" / f"{step}.txt").write_text(ans)
    prov["steps"][step] = {"model": CFG["models"][key]["model"], "seed": seed, "usage": usage}
    f = files_in(ans)
    st = prov["state"]
    if st == "kernel":
        missing = [n for n in ("spec.md", "reference.hpp", "stub.hpp", "test.cpp") if n not in f]
        if missing:
            prov["rejected"], prov["state"] = "format: missing " + ", ".join(missing), "done"
            return
        f["test.cpp"], prov["test_include_normalized"] = normalize_test(f["test.cpp"])
        for n in ("spec.md", "reference.hpp", "stub.hpp", "test.cpp"):
            (kdir / n).write_text(f[n])
        prov["state"] = "second"
    elif st == "second":
        if "reference.hpp" not in f:
            prov["rejected"], prov["state"] = "format: second reference missing", "done"
            return
        (kdir / "reference_2.hpp").write_text(f["reference.hpp"])
        prov["state"] = "variants"
    elif st == "variants":
        (kdir / "variants").mkdir(exist_ok=True)
        if "variant.hpp" in f:
            (kdir / "variants" / f"{step[len('variant_'):]}.hpp").write_text(f["variant.hpp"])
        if all(f"variant_{p['name']}" in prov["steps"] for p in variant_patterns(prov)):
            prov["state"] = "gates"
    elif st == "repair":
        changed = []
        if step.endswith("_second") and "reference.hpp" in f:
            (kdir / "reference_2.hpp").write_text(f["reference.hpp"])
            changed.append("reference_2.hpp")
        if step.endswith("_kernel"):
            if "reference.hpp" in f:
                (kdir / "reference.hpp").write_text(f["reference.hpp"])
                changed.append("reference.hpp")
            if "test.cpp" in f:
                t, norm = normalize_test(f["test.cpp"])
                prov["test_include_normalized"] = prov.get("test_include_normalized") or norm
                (kdir / "test.cpp").write_text(t)
                changed.append("test.cpp")
        prov.setdefault("repairs", []).append(
            {"round": prov["round"], "after": prov["rejected"], "by": key, "files": changed})
        if changed:
            prov.setdefault("gates_history", []).append({"gates": prov["gates"], "rejected": prov["rejected"]})
            prov["gates"], prov["rejected"] = {}, None
            shutil.rmtree(kdir / "mutants", ignore_errors=True)
            prov["state"] = "gates"
        else:
            prov["state"] = "done"  # nothing usable came back: the earlier rejection stands


def gate_step(kdir: Path, prov: dict) -> None:
    """State gates: run G1 to G3; on a repairable failure go to repair while rounds are left."""
    if gates(kdir, prov):
        prov["state"] = "done"
    elif prov["round"] < CFG.get("repair_rounds", 0) and (prov["rejected"] or "").startswith(("G1", "G2")):
        prov["round"] += 1
        prov["state"] = "repair"
    else:
        prov["state"] = "done"


def finish(prov: dict) -> dict:
    if prov["state"] == "done" and "finished" not in prov:
        prov["accepted"] = prov["rejected"] is None
        prov["finished"] = time.time()
    return prov


def generate_one(idea: dict, out: Path) -> dict:
    """One candidate from start to end with the HTTP endpoints (GEN_URL_31B / GEN_URL_12B)."""
    kdir, prov = candidate(idea, out)
    while prov["state"] != "done":
        try:
            if prov["state"] == "gates":
                gate_step(kdir, prov)
            else:
                reqs = requests_of(kdir, prov)
                if not reqs:
                    prov["state"] = "done"
                for r in reqs:
                    seed = seed_of(prov["id"], r["step"])
                    ans, usage = chat(r["key"], r["prompt"], seed)
                    apply_answer(kdir, prov, r, ans, usage, seed)
        except Exception as e:  # noqa: BLE001  keep going, record why
            prov["rejected"], prov["state"] = f"error: {e!r}"[:500], "done"
        finish(prov)
        save(kdir, prov)
    return prov


def llm_batch(key: str, reqs: list[dict], work: Path) -> dict[str, tuple[str, dict]]:
    """Answer many prompts with one model loaded in-process (synthetic/generator/llm_batch.py in a subprocess, which frees
    the GPU when it exits). No server, no network. reqs: [{"id", "prompt", "seed"}] -> {id: (text, usage)}."""
    work.mkdir(parents=True, exist_ok=True)
    tag = f"{time.strftime('%Y%m%d-%H%M%S')}_{os.getpid()}_{key}"  # unique across parallel array jobs
    rf, af = work / f"{tag}.requests.jsonl", work / f"{tag}.answers.jsonl"
    rf.write_text("".join(json.dumps({"id": r["id"], "system": prompts.SYSTEM, "user": r["prompt"],
                                      "seed": r["seed"]}) + "\n" for r in reqs))
    runner = os.environ.get("GEN_LLM_RUNNER", str(ROOT / "synthetic/generator/llm_batch.py"))
    cmd = [sys.executable, runner, "--model", CFG["models"][key]["model"], "--requests", str(rf),
           "--answers", str(af), "--temperature", str(SAMPLING["temperature"]), "--top-p", str(SAMPLING["top_p"]),
           "--top-k", str(SAMPLING["top_k"]), "--max-tokens", str(SAMPLING["max_tokens"])]
    print(f"{time.strftime('%H:%M:%S')} {key}: {len(reqs)} prompts", flush=True)
    t0 = time.time()
    for attempt in range(1, 4):  # a model start can fail on a transient file system error (BeeGFS under load)
        if subprocess.run(cmd).returncode == 0 and af.exists():
            break
        if attempt == 3:
            raise RuntimeError(f"llm_batch for {key} failed 3 times; see the job log")
        wait = int(os.environ.get("GEN_RETRY_WAIT", "90")) * attempt
        print(f"{time.strftime('%H:%M:%S')} {key}: llm_batch failed (attempt {attempt} of 3), retrying in "
              f"{wait} s", flush=True)
        time.sleep(wait)
    res = {}
    for line in af.read_text().splitlines():
        d = json.loads(line)
        res[d["id"]] = (d["text"], d.get("usage", {}))
    print(f"{time.strftime('%H:%M:%S')} {key}: {len(res)} answers in {time.time() - t0:.0f} s", flush=True)
    return res


def run_offline(ideas: list[dict], out: Path, workers: int) -> list[dict]:
    """All candidates together: per cycle, every request of the 31B in one batch, then of the 12B, then
    the gates on the CPU, until every candidate is done."""
    cands = [candidate(i, out) for i in ideas]
    for cycle in range(1, 20):
        for key in ("31b", "12b"):
            pending = []
            for kdir, prov in cands:
                if prov["state"] in ("done", "gates"):
                    continue
                reqs = requests_of(kdir, prov)
                if not reqs:
                    prov["state"] = "done"
                    finish(prov)
                    save(kdir, prov)
                pending += [(kdir, prov, r) for r in reqs if r["key"] == key]
            if not pending:
                continue
            batch = [{"id": f"{prov['id']}/{r['step']}", "prompt": r["prompt"],
                      "seed": seed_of(prov["id"], r["step"])} for _, prov, r in pending]
            answers = llm_batch(key, batch, out / "llm")
            for (kdir, prov, r), b in zip(pending, batch):
                try:
                    ans, usage = answers.get(b["id"], ("", {}))
                    apply_answer(kdir, prov, r, ans, usage, b["seed"])
                except Exception as e:  # noqa: BLE001
                    prov["rejected"], prov["state"] = f"error: {e!r}"[:500], "done"
                finish(prov)
                save(kdir, prov)
        in_gates = [(k, p) for k, p in cands if p["state"] == "gates"]

        def gate_one(kp):
            kdir, prov = kp
            try:
                gate_step(kdir, prov)
            except Exception as e:  # noqa: BLE001
                prov["rejected"], prov["state"] = f"error: {e!r}"[:500], "done"
            finish(prov)
            save(kdir, prov)

        with cf.ThreadPoolExecutor(workers) as ex:
            list(ex.map(gate_one, in_gates))
        left = Counter(p["state"] for _, p in cands)
        print(f"{time.strftime('%H:%M:%S')} cycle {cycle}: " + ", ".join(f"{k} {v}" for k, v in sorted(left.items())),
              flush=True)
        if left.get("done", 0) == len(cands):
            break
    return [p for _, p in cands]


def offline() -> bool:
    return os.environ.get("GEN_LLM", "http") == "offline"


def get_ideas(out: Path, domains: list[dict], levels: list[str], workers: int) -> list[dict]:
    ifile = out / "ideas.jsonl"
    if ifile.exists():
        return [json.loads(l) for l in ifile.read_text().splitlines() if l.strip()]
    n = CFG["ideas_per_domain_per_level_per_model"]
    jobs = [(d, lv, key) for d in domains for lv in levels for key in ("31b", "12b")]

    def prompt(job):
        d, lv, key = job
        return prompts.IDEAS.format(domain=d["name"], about=d["about"], n=n, taken="", level=lv,
                                    level_about=LEVELS[lv]["about"])

    def parse(job, ans):
        d, lv, key = job
        (out / "ideas_raw").mkdir(parents=True, exist_ok=True)
        (out / "ideas_raw" / f"{d['name']}__{lv}__{key}.txt").write_text(ans)
        res = []
        for line in ans.splitlines():
            line = line.strip().strip(",")
            if not line.startswith("{"):
                continue
            try:
                o = json.loads(line)
            except json.JSONDecodeError:
                continue
            if re.fullmatch(r"[a-z][a-z0-9_]{2,40}", str(o.get("name", ""))) and \
                    re.fullmatch(r"[A-Z][A-Za-z0-9]{2,40}", str(o.get("class", ""))):
                res.append({"domain": d["name"], "level": lv, "name": o["name"], "class": o["class"],
                            "summary": str(o.get("summary", ""))[:300], "generator": key})
        return res[:n]

    def seed(job):
        return seed_of("ideas", job[0]["name"], job[1], job[2])

    if offline():
        answers = {}
        for key in ("31b", "12b"):
            mine = [j for j in jobs if j[2] == key]
            got = llm_batch(key, [{"id": str(i), "prompt": prompt(j), "seed": seed(j)}
                                  for i, j in enumerate(jobs) if j[2] == key], out / "llm")
            answers.update({jobs.index(j): got[str(jobs.index(j))][0] for j in mine})
        results = [parse(j, answers[i]) for i, j in enumerate(jobs)]
    else:
        with cf.ThreadPoolExecutor(workers) as ex:
            results = list(ex.map(lambda j: parse(j, chat(j[2], prompt(j), seed(j))[0]), jobs))

    ideas, seen = [], set()
    for res in results:
        for i in res:  # a model keeps its own idea even if the other proposed the same name
            k = (i["domain"], i["level"], i["name"], i["generator"])
            if k not in seen:
                seen.add(k)
                ideas.append(i)
    out.mkdir(parents=True, exist_ok=True)
    ifile.write_text("".join(json.dumps(i) + "\n" for i in ideas))
    return ideas


def _scope(a):
    domains = [d for d in CFG["domains"] if not a.domains or d["name"] in a.domains.split(",")]
    levels = [lv for lv in LEVELS if not a.levels or lv in a.levels.split(",")]
    return domains, levels


def cmd_ideas(a) -> None:
    out = Path(a.out).resolve()
    domains, levels = _scope(a)
    ideas = get_ideas(out, domains, levels, a.workers)
    print(f"{len(ideas)} ideas: " + ", ".join(f"{k} {v}" for k, v in Counter(i["level"] for i in ideas).items()))


def cmd_run(a) -> None:
    out = Path(a.out).resolve()
    domains, levels = _scope(a)
    if a.shard:
        if not (out / "ideas.jsonl").exists():
            sys.exit("sharded run: create the ideas first (synthetic/generator/generate.py ideas --out ...)")
        k, n = map(int, a.shard.split("/"))
    ideas = get_ideas(out, domains, levels, a.workers)
    ideas = [i for i in ideas if i["domain"] in {d["name"] for d in domains} and i["level"] in levels]
    if a.shard:
        ideas = [i for i in ideas if seed_of(i["domain"], i["level"], i["name"], i["generator"]) % n == k]
    if a.limit:
        ideas = ideas[:a.limit]
    print(f"{len(ideas)} ideas ({'offline, in-process model' if offline() else 'HTTP endpoints'})", flush=True)
    if offline():
        run_offline(ideas, out, a.workers)
    else:
        done = Counter()
        with cf.ThreadPoolExecutor(a.workers) as ex:
            for prov in ex.map(lambda i: generate_one(i, out), ideas):
                with LOCK:
                    done["accepted" if prov.get("accepted") else (prov.get("rejected") or "?").split(":")[0]] += 1
                    print(f"[{sum(done.values())}/{len(ideas)}] {prov['id']}: "
                          f"{'ACCEPTED' if prov.get('accepted') else prov.get('rejected')}", flush=True)
    cmd_report(a)


def cmd_report(a) -> None:
    out = Path(a.out)
    provs = [json.loads(p.read_text()) for p in sorted(out.glob("candidates/*/provenance.json"))]
    stage = Counter()
    by_gen = Counter()
    by_level = Counter()
    for p in provs:
        if p.get("state", "done") != "done":
            stage["open"] += 1  # not finished yet (rejected is None until then)
        else:
            stage[(p.get("rejected") or "accepted").split(":")[0]] += 1
        if p.get("accepted"):
            by_gen[p["generator"]] += 1
            by_level[p["idea"].get("level", "?")] += 1
    print(f"{len(provs)} candidates; outcome per stage: " + ", ".join(f"{k} {v}" for k, v in stage.most_common()))
    print("accepted per generator: " + ", ".join(f"{k} {v}" for k, v in by_gen.items()))
    print("accepted per level: " + ", ".join(f"{k} {by_level[k]}" for k in LEVELS))
    nv = sum(len(p["gates"].get("G3", {}).get("kept", [])) for p in provs if p.get("accepted"))
    print(f"tasks: {sum(by_gen.values())} implement + {nv} remediate")
    open_ = Counter(p.get("state", "done") for p in provs if p.get("state", "done") != "done")
    if open_:
        print("not finished: " + ", ".join(f"{k} {v}" for k, v in sorted(open_.items())))

    def outcome(p):
        r = p.get("rejected")
        if p.get("state", "done") != "done":
            return "open"
        if r is None:
            return "accepted"
        if r.startswith("G1"):
            g1 = p["gates"].get("G1", {}).values()
            return "G1 build" if any(x and x.get("build") == 0 for x in g1) else "G1 test"
        if r.startswith("G2"):
            return "G2 stub" if "stub" in r else "G2 mutants"
        return r.split(":")[0]

    cols = ["accepted", "G1 build", "G1 test", "G2 stub", "G2 mutants", "G3", "format", "error", "open"]
    gen_tag = lambda m: "31B" if "31B" in m else "12B" if "12B" in m else m  # noqa: E731
    rows = {}
    for p in provs:
        for key in [(gen_tag(p["generator"]), p["idea"].get("level", "?")), (gen_tag(p["generator"]), "all"),
                    ("both", p["idea"].get("level", "?")), ("both", "all")]:
            r = rows.setdefault(key, Counter())
            r["n"] += 1
            r[outcome(p)] += 1
            if p.get("accepted"):
                r["after repair"] += bool(p.get("repairs"))
                r["implement"] += 1
                r["remediate"] += len(p["gates"].get("G3", {}).get("kept", []))
    order = {lv: i for i, lv in enumerate(list(LEVELS) + ["?", "all"])}
    print("\n| Generator | Level | Candidates | Accepted (%) | after repair | " + " | ".join(cols[1:]) +
          " | Implement | Remediate |")
    print("|---|---|---|---|---|" + "---|" * (len(cols) - 1) + "---|---|")
    for (g, lv) in sorted(rows, key=lambda k: (k[0] == "both", k[0], order.get(k[1], 9))):
        r = rows[(g, lv)]
        print(f"| {g} | {lv} | {r['n']} | {r['accepted']} ({100 * r['accepted'] / r['n']:.0f}) | {r['after repair']} | "
              + " | ".join(str(r[c]) for c in cols[1:]) + f" | {r['implement']} | {r['remediate']} |")
    structure(provs, out)
    if getattr(a, "latex", None):
        write_latex(rows, Path(a.latex))
        print(f"LaTeX table -> {a.latex}")
    if getattr(a, "fig", None):
        write_fig(rows, Path(a.fig))
        print(f"LaTeX figure -> {a.fig}")


def structure(provs: list[dict], out: Path) -> None:
    """Are the two generator halves alike? Medians per generator and level over accepted kernels: reference
    lines, real-time methods (public methods of the stub besides configure), checks and guarded calls in the
    test, mutant kill rate, kept variants."""
    import statistics
    cols = defaultdict(lambda: defaultdict(list))
    for p in provs:
        if not p.get("accepted"):
            continue
        d = out / "candidates" / p["id"]
        ref, stub, test = ((d / f).read_text() for f in ("reference.hpp", "stub.hpp", "test.cpp"))
        methods = {m for m in re.findall(r"^\s*(?:[\w:<>,\s\*&]+?)\s+(\w+)\s*\([^;{]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{",
                                         stub, re.M) if m not in ("configure", "if", "for", "while", "switch")}
        g = "31B" if "31B" in p["generator"] else "12B"
        for key in [(g, p["idea"].get("level", "?")), (g, "all")]:
            c = cols[key]
            c["reference lines"].append(sum(1 for l in ref.splitlines() if l.strip()))
            c["real-time methods"].append(len(methods))
            c["checks in test"].append(len(re.findall(r"rtk::expect|rtk::near\(run", test)))
            c["guarded call sites"].append(len(re.findall(r"rtk::guarded", test)))
            c["kill rate %"].append(100 * p["gates"].get("G2", {}).get("kill_rate", 0))
            c["kept variants"].append(len(p["gates"].get("G3", {}).get("kept", [])))
    if not cols:
        return
    names = ["reference lines", "real-time methods", "checks in test", "guarded call sites", "kill rate %",
             "kept variants"]
    order = {lv: i for i, lv in enumerate(list(LEVELS) + ["?", "all"])}
    print("\nStructure of accepted kernels (medians):\n")
    print("| Generator | Level | Kernels | " + " | ".join(names) + " |")
    print("|---|---|---|" + "---|" * len(names))
    for (g, lv) in sorted(cols, key=lambda k: (k[0], order.get(k[1], 9))):
        c = cols[(g, lv)]
        print(f"| {g} | {lv} | {len(c['reference lines'])} | "
              + " | ".join(f"{statistics.median(c[n]):.0f}" for n in names) + " |")


def write_latex(rows: dict, path: Path) -> None:
    """The yield table of the paper: per generator and level, candidates, where the rest failed, accepted
    (and how many only after a repair round), and the tasks they give."""
    def pct(x, n):
        return f"{x}\\,{{\\scriptsize\\textcolor{{black!55}}{{({100 * x / n:.0f}\\%)}}}}" if n else "--"

    lines = ["% generated by synthetic/generator/generate.py report --latex; do not edit by hand",
             "\\begin{tabular}{@{}llrrrrrrrr@{}}", "  \\toprule",
             "  & & & \\multicolumn{4}{c}{Rejected at} & \\multicolumn{3}{c}{Accepted} \\\\",
             "  \\cmidrule(lr){4-7}\\cmidrule(l){8-10}",
             "  Generator & Level & Cand. & G1 & G2 & G3 & other & Kernels & via repair & Remed. \\\\",
             "  \\midrule"]
    gens = [g for g in ("31B", "12B", "both") if any(k[0] == g for k in rows)]
    for gi, g in enumerate(gens):
        lvls = [lv for lv in list(LEVELS) + ["all"] if (g, lv) in rows]
        for li, lv in enumerate(lvls):
            r = rows[(g, lv)]
            g1, g2 = r["G1 build"] + r["G1 test"], r["G2 stub"] + r["G2 mutants"]
            other = r["format"] + r["error"] + r["open"]
            name = ("Gemma~4 " + g if g != "both" else "Both") if li == 0 else ""
            lvname = "\\textit{all}" if lv == "all" else lv
            lines.append(f"  {name} & {lvname} & {r['n']} & {g1} & {g2} & {r['G3']} & {other} & "
                         f"{pct(r['accepted'], r['n'])} & {r['after repair']} & {r['remediate']} \\\\")
        if gi < len(gens) - 1:
            lines.append("  \\midrule")
    lines += ["  \\bottomrule", "\\end{tabular}"]
    path.write_text("\n".join(lines) + "\n")


def write_fig(rows: dict, path: Path) -> None:
    """The generation figure of the paper: the pipeline in the visual language of figures/overview.tex (blue:
    a model writes; orange: the dual verifier; green: accepted), with the number of candidates left after each
    step on the arrows and the number each gate rejects below it. Counts come from `rows` (all generators and
    levels together), so the figure always matches the data."""
    r = rows[("both", "all")]
    other = r["format"] + r["error"] + r["open"]
    g1, g2, g3 = r["G1 build"] + r["G1 test"], r["G2 stub"] + r["G2 mutants"], r["G3"]
    left = [r["n"], r["n"] - other]
    left += [left[-1] - g1, left[-1] - g1 - g2, left[-1] - g1 - g2 - g3]
    assert left[-1] == r["accepted"], (left, r["accepted"])
    per_gen = [(g, rows[(g, "all")]["accepted"], rows[(g, "all")]["n"]) for g in ("31B", "12B") if (g, "all") in rows]
    xs = [0.95, 3.25, 5.55, 7.85, 10.15]
    gen_lines = " \\\\ ".join(f"{g}: {a} of {n}" for g, a, n in per_gen)
    yr = 1.02  # repair line
    path.write_text(f"""% generated by synthetic/generator/generate.py report --fig; do not edit by hand
% Synthetic task generation: the pipeline (same visual language as figures/overview.tex: blue = a model writes,
% orange = the dual verifier, green = accepted, grey = rejected) with the candidates left after each step on
% the arrows and the rejections below each gate. Designed for the 5.5in (13.97cm) NeurIPS text width.
\\begin{{tikzpicture}}[
  >={{Latex[length=3.5pt,width=2.6pt]}},
  box/.style={{line width=0.5pt, rounded corners=1pt, align=center, font=\\scriptsize, inner sep=2pt,
    minimum height=1.5cm, text width=1.75cm}},
  model/.style={{box, draw=rtblue, fill=rtbluelight}},
  gate/.style={{box, draw=rtred, fill=rtredlight}},
  arr/.style={{->, line width=0.55pt, draw=black!80}},
  cnt/.style={{font=\\scriptsize, inner sep=1pt, above=1.5pt}},
  rej/.style={{font=\\scriptsize, text=black!55, inner sep=1pt, anchor=north}},
  rep/.style={{->, line width=0.45pt, draw=black!50}},
]
\\newcommand{{\\gdim}}[1]{{{{\\color{{black!62}}#1}}}}
\\node[model] (gen) at ({xs[0]},0) {{\\textbf{{Generator}}\\\\\\gdim{{31B or 12B}}\\\\spec, reference,\\\\stub, test}};
\\node[model] (oth) at ({xs[1]},0) {{\\textbf{{Other model}}\\\\\\gdim{{12B or 31B}}\\\\second reference\\\\from the spec}};
\\node[gate] (g1) at ({xs[2]},0) {{\\textbf{{G1}}\\\\both references\\\\pass the test,\\\\no allocation}};
\\node[gate] (g2) at ({xs[3]},0) {{\\textbf{{G2}}\\\\test rejects the\\\\stub and ${{\\geq}}80\\%$\\\\of mutants}};
\\node[gate] (g3) at ({xs[4]},0) {{\\textbf{{G3}}\\\\an allocating\\\\variant still\\\\passes the test}};
\\node[anchor=west, align=left, font=\\scriptsize, inner sep=0pt] (out) at ({xs[4] + 1.43:.2f},0)
  {{{{\\color{{rtgreen!60!black}}\\textbf{{{r['accepted']} kernels}}}}\\\\{r['implement']} implementations\\\\{r['remediate']} allocation removals}};
\\draw[arr] (gen) -- node[cnt] {{{left[0]}}} (oth);
\\draw[arr] (oth) -- node[cnt] {{{left[1]}}} (g1);
\\draw[arr] (g1) -- node[cnt] {{{left[2]}}} (g2);
\\draw[arr] (g2) -- node[cnt] {{{left[3]}}} (g3);
\\draw[arr] (g3) -- node[cnt] {{{left[4]}}} (out.west);
\\node[rej] at (g1.south) [yshift=-2pt] {{$-${g1}}};
\\node[rej] at (g2.south) [yshift=-2pt] {{$-${g2}}};
\\node[rej] at (g3.south) [yshift=-2pt] {{$-${g3}}};
\\node[rej] at ({(xs[0] + xs[1]) / 2:.2f},-0.78) {{{other} malformed}};
\\node[rej, anchor=north west, align=left] at ({xs[4] + 1.43:.2f},-0.5) {{{gen_lines}}};
% repair loop: a failed G1 or G2 goes back to the model that wrote the failing file
\\draw[line width=0.45pt, draw=black!50] (g2.north) -- ({xs[3]},{yr}) -- ({xs[0]},{yr});
\\draw[rep] ({xs[0]},{yr}) -- (gen.north);
\\draw[rep] ({xs[1]},{yr}) -- (oth.north);
\\node[font=\\scriptsize, text=black!55, anchor=south, inner sep=1.5pt] at ({(xs[0] + xs[3]) / 2:.2f},{yr})
  {{repair: a failed G1 or G2 goes back to the author of the file, at most twice ({r['after repair']} of {r['accepted']})}};
\\end{{tikzpicture}}
""")


def cmd_regate(a) -> None:
    """Run the gates again on existing candidates (after a fix to the gates or the test helper),
    without asking any model. Rejected variants are put back first; the old gate results are kept
    in provenance under "previous"."""
    out = Path(a.out).resolve()
    kdirs = [p.parent for p in sorted(out.glob("candidates/*/provenance.json"))]

    def one(kdir: Path) -> dict:
        prov = json.loads((kdir / "provenance.json").read_text())
        if not (kdir / "test.cpp").exists() or not (kdir / "reference_2.hpp").exists():
            return prov  # rejected before the gates (format); nothing to redo
        prov.setdefault("previous", []).append({"gates": prov["gates"], "rejected": prov["rejected"]})
        t, changed = normalize_test((kdir / "test.cpp").read_text())
        if changed:
            (kdir / "test.cpp").write_text(t)
            prov["test_include_normalized"] = True
        rv = kdir / "rejected_variants"
        if rv.exists():
            for v in rv.glob("*.hpp"):
                shutil.move(str(v), kdir / "variants" / v.name)
            rv.rmdir()
        shutil.rmtree(kdir / "mutants", ignore_errors=True)
        prov["gates"], prov["rejected"] = {}, None
        try:
            gates(kdir, prov)
        except Exception as e:  # noqa: BLE001
            prov["rejected"] = f"error: {e!r}"[:500]
        prov["accepted"] = prov["rejected"] is None
        prov["regated"] = time.time()
        (kdir / "provenance.json").write_text(json.dumps(prov, indent=1))
        return prov

    with cf.ThreadPoolExecutor(a.workers) as ex:
        for i, prov in enumerate(ex.map(one, kdirs), 1):
            print(f"[{i}/{len(kdirs)}] {prov['id']}: {'ACCEPTED' if prov.get('accepted') else prov.get('rejected')}",
                  flush=True)
    cmd_report(a)


# ----------------------------------------------------------------------------- self-test
def cmd_selftest(a) -> None:
    """Run the gates on the hand-written fixtures (reference used as both references): every fixture
    must pass; a copy whose test checks nothing must fail G2; a copy without allocating variants G3."""
    ok = True
    with tempfile.TemporaryDirectory(dir=ROOT / "synthetic" / "generator") as tmp:
        tmp = Path(tmp)
        for k in sorted((ROOT / "synthetic/fixtures").iterdir()):
            d = tmp / k.name
            shutil.copytree(k, d)
            shutil.copy(d / "reference.hpp", d / "reference_2.hpp")
            prov = {"id": k.name, "gates": {}, "rejected": None}
            acc = gates(d, prov)
            g2 = prov["gates"].get("G2", {})
            print(f"{k.name:16s} {'accepted' if acc else 'REJECTED ' + prov['rejected']}"
                  f"  (mutants {g2.get('compiled')}/{g2.get('mutants')} compile, kill rate {g2.get('kill_rate')})")
            ok &= acc
        d = tmp / "weak"
        shutil.copytree(ROOT / "synthetic/fixtures/moving_average", d)
        shutil.copy(d / "reference.hpp", d / "reference_2.hpp")
        t = (d / "test.cpp").read_text()
        (d / "test.cpp").write_text(re.sub(r"rtk::expect\(run, ", "rtk::expect(run, true || ", t))
        prov = {"id": "weak", "gates": {}, "rejected": None}
        weak = gates(d, prov)
        print(f"{'weak test':16s} {'accepted (WRONG)' if weak else 'rejected as expected: ' + prov['rejected']}")
        ok &= not weak
    print("selftest passed" if ok else "selftest FAILED")
    sys.exit(0 if ok else 1)


def main() -> None:
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("--out", default="synthetic/generator/out/run")
    r.add_argument("--workers", type=int, default=16)
    r.add_argument("--domains", default="")
    r.add_argument("--levels", default="", help="comma-separated subset of easy,medium,hard")
    r.add_argument("--limit", type=int, default=0)
    r.add_argument("--shard", default="", help="k/n: this job takes the ideas with hash %% n == k")
    i = sub.add_parser("ideas")
    i.add_argument("--out", default="synthetic/generator/out/run")
    i.add_argument("--workers", type=int, default=16)
    i.add_argument("--domains", default="")
    i.add_argument("--levels", default="")
    sub.add_parser("selftest")
    rg = sub.add_parser("regate")
    rg.add_argument("--out", default="synthetic/generator/out/run")
    rg.add_argument("--workers", type=int, default=4)
    rp = sub.add_parser("report")
    rp.add_argument("--out", default="synthetic/generator/out/run")
    rp.add_argument("--latex", default="", help="also write the yield table for the paper to this file")
    rp.add_argument("--fig", default="", help="also write the yield figure (pgfplots) for the paper to this file")
    a = ap.parse_args()
    {"run": cmd_run, "ideas": cmd_ideas, "selftest": cmd_selftest, "report": cmd_report,
     "regate": cmd_regate}[a.cmd](a)


if __name__ == "__main__":
    main()
