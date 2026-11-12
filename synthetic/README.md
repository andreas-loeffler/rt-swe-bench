# Synthetic tasks

303 tasks on 109 self-contained C++ real-time kernels, written by Gemma 4 (31B and 12B) and kept only when the
dual verifier confirms them. Each kernel has a real-time entry point (`process()`, `push()`, `update()`, ...) that
must not allocate and a setup call (`configure()`) that may.

```
dataset/              the dataset
  kernels/<id>/       spec.md, reference.hpp, reference_2.hpp (independent second reference by the other
                      model), stub.hpp, test.cpp, variants/<pattern>.hpp (allocating, passes the test),
                      provenance.json (generator, prompt version, sampling, gate results, repair rounds)
  candidates.jsonl    all 472 candidates with the gate that rejected the rest
generator/            the generator: generate.py (ideas, generation, gates G1-G3, repair, report),
                      prompts.py, mutate.py (mutants for G2), config.json (domains, levels, patterns),
                      export_dataset.py, audit.py, llm_batch.py (models in-process on a cluster)
fixtures/             hand-written kernels for the generator's self-test (not dataset content)
include/              shared test helpers
score.sh              score one candidate: F (the test passes) and R (no allocation in a guarded call)
validate.sh           validate kernels: reference F=1 R=1, variants F=1 R=0, stub F=0
```

Tasks per kernel: one implementation task (start from `stub.hpp`) and one remediation task per allocating
variant (start from `variants/<pattern>.hpp`).

```
synthetic/score.sh synthetic/dataset/kernels/<id> <candidate.hpp>        # in the image or a native setup
synthetic/validate.sh moving_average                                     # a fixture, in the image
python3 synthetic/generator/generate.py selftest                         # the gates on the fixtures
python3 synthetic/generator/generate.py run --out synthetic/generator/out/run   # needs the two Gemma 4 servers
python3 synthetic/generator/export_dataset.py synthetic/generator/out/run/candidates synthetic/dataset
python3 scripts/agent_synthetic.py report --runs results/synthetic_runs  # agent results (paper, Table 5)
```
