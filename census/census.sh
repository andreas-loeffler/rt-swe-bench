#!/usr/bin/env bash
# Build the rtswe_eval probes in the pinned image, run them, and collect one
# JSON line per probe into results/census/census.jsonl (plus the full log).
#   census/census.sh                    # all probes
#   census/census.sh probe_pid_controller
set -euo pipefail
cd "$(dirname "$0")/.."

IMAGE="${IMAGE:-rt-swe-bench/base:jazzy-d71703b}"
OUT="results/census"
mkdir -p "$OUT"
FILTER="${1:-probe_}"

docker run --rm \
  -v "$PWD/harness/rtswe_eval:/ws/src/rtswe_eval:ro" \
  "$IMAGE" bash -c "
    set -e
    cd /ws
    colcon build --packages-select rtswe_eval \
      --event-handlers console_direct- \
      --cmake-args -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON > /tmp/build.log 2>&1 \
      || { tail -60 /tmp/build.log; exit 1; }
    colcon test --packages-select rtswe_eval --ctest-args -R '${FILTER}' > /dev/null 2>&1 || true
    cat /ws/log/latest_test/rtswe_eval/stdout_stderr.log
  " > "$OUT/census.log"

# ctest prefixes each line with "<test number>: ".
sed -n 's/^[0-9]*: \[rtswe-json\] //p' "$OUT/census.log" > "$OUT/census.jsonl"
sed -n 's/^[0-9]*: \(\[rtswe\] \(probe=\|cycles=\|site:\)\)/\1/p' "$OUT/census.log"
echo "-> $OUT/census.jsonl ($(wc -l < "$OUT/census.jsonl") probes)"
