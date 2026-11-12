#!/usr/bin/env bash
# Reproduce the task validation of RT-SWE-Bench in the pinned container.
#
# For each task, the dual verifier (functional tests + malloc-guard around
# update()) scores three versions of the code: the task's starting point, the
# reference patch and the naive patch. The verdicts must match what the task's
# track requires (below) and the results recorded in tasks/<id>/validation.jsonl.
#
#               Track 1 (bug fix)   Track 2 (RT remediation)
#   unpatched   F=0                 F=1, R=0
#   reference   F=1, R=1            F=1, R=1
#   naive       F=1, R=0            F=1, R=0
#
#   ./verify.sh                 # rt-001: about a minute once the image exists
#   ./verify.sh rt-006 rt-014   # chosen tasks
#   ./verify.sh all             # every task (about an hour)
#
# Needs Docker and the image rt-swe-bench/base:jazzy-d71703b, built once with
# docker/build.sh (about 30 to 60 minutes, 16 GB RAM or more). Without Docker,
# scripts/setup_native.sh builds the same environment on Ubuntu 24.04; then run
# with RTSWE_BACKEND=native.
set -euo pipefail
cd "$(dirname "$0")"
IMAGE="${IMAGE:-rt-swe-bench/base:jazzy-d71703b}"

if [[ "${RTSWE_BACKEND:-docker}" == docker ]]; then
  command -v docker > /dev/null || { echo "docker not found (or set RTSWE_BACKEND=native)"; exit 1; }
  docker image inspect "$IMAGE" > /dev/null 2>&1 \
    || { echo "image $IMAGE not found: build it once with docker/build.sh"; exit 1; }
fi

tasks=("$@")
[[ ${#tasks[@]} -eq 0 ]] && tasks=(rt-001)
[[ "${tasks[0]}" == all ]] && mapfile -t tasks < <(ls tasks | grep '^rt-' | sort)

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
failed=()
for t in "${tasks[@]}"; do
  dir="tasks/$t"
  [[ -f "$dir/task.json" ]] || { echo "no such task: $t"; failed+=("$t"); continue; }
  python3 - "$dir/task.json" <<'PY'
import json, sys
t = json.load(open(sys.argv[1]))
print(f"== {t['id']}  track {t['track']}  {t['package']}  ({t['source']})")
PY
  if ! VALIDATION_OUT="$tmp/$t.jsonl" scripts/validate_task.sh "$dir"; then
    failed+=("$t")
    continue
  fi
  # same verdicts as recorded in the repository?
  if ! python3 - "$dir/validation.jsonl" "$tmp/$t.jsonl" <<'PY'
import json, sys
load = lambda p: {r["variant"]: (r["F"], r["R"]) for r in map(json.loads, open(p))}
recorded, now = load(sys.argv[1]), load(sys.argv[2])
diff = {v: (recorded.get(v), now.get(v)) for v in now if recorded.get(v) != now.get(v)}
print("matches the recorded validation" if not diff else f"differs from the recorded validation: {diff}")
sys.exit(1 if diff else 0)
PY
  then
    failed+=("$t")
  fi
done

echo
if [[ ${#failed[@]} -eq 0 ]]; then
  echo "verified: ${tasks[*]}"
else
  echo "FAILED: ${failed[*]}"
  exit 1
fi
