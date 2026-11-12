#!/usr/bin/env bash
# Score one candidate implementation of a synthetic kernel (inside the benchmark
# image or a native setup with malloc-guard in $MALLOC_GUARD, default /opt/malloc_guard).
#   synthetic/score.sh synthetic/fixtures/<name> <candidate.hpp>   -> one JSON line
# The candidate is compiled as kernel.hpp into the kernel's test (RelWithDebInfo
# flags, like the repository tier) and run once.
# F: it builds and the test's functional check passes.
# R: no allocation in any guarded real-time call.
set -u
MG=${MALLOC_GUARD:-/opt/malloc_guard}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DIR=$(cd "$1" && pwd); CAND=$2
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
cp "$CAND" "$TMP/kernel.hpp"
build=1; F=0; R=0; json=null
${CXX:-g++} -std=c++20 -O2 -g -fno-omit-frame-pointer -Wall -I "$TMP" -I "$ROOT/synthetic/include" \
  -I "$ROOT/harness/rtswe_eval/include" -I "$MG/include" "$DIR/test.cpp" -o "$TMP/test" \
  -L "$MG/lib" -lmalloc_guard -ldl > "$TMP/build.log" 2>&1 || build=0
if [[ $build == 1 ]]; then
  LD_LIBRARY_PATH="$MG/lib" timeout 60 "$TMP/test" > "$TMP/run.log" 2>&1
  func=$(sed -n 's/^\[rtswe\] functional_ok=\([01]\).*/\1/p' "$TMP/run.log" | head -1)
  json=$(sed -n 's/^\[rtswe-json\] //p' "$TMP/run.log" | head -1)
  [[ -z "$json" ]] && json=null
  [[ "$func" == 1 ]] && F=1
  [[ "$json" != null ]] && grep -q '"allocating_cycles":0,' <<< "$json" && R=1
fi
err=""
[[ $build == 0 ]] && err=$(grep -m3 "error" "$TMP/build.log" | tr '"\\' "''" | tr '\n' ' ')
mismatch=$(sed -n 's/^\[rtswe\] functional_ok=0 first_mismatch=\(.*\)/\1/p' "$TMP/run.log" 2>/dev/null | head -1 | tr '"\\' "''")
printf '{"build":%s,"F":%s,"R":%s,"alloc":%s,"mismatch":"%s","build_error":"%s"}\n' \
  "$build" "$F" "$R" "$json" "$mismatch" "$err"
