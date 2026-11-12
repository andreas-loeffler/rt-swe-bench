#!/usr/bin/env bash
# Score one patch inside a prepared workspace ($RTSWE_WS, default /ws, with
# ros2_controllers at the pinned commit and rtswe_eval next to it), with the ROS
# environment sourced.
# Called by evaluate.sh, inside the container or natively.
#   score_in_ws.sh PKG PROBE PATCH|none LOGDIR [BASE|none] [TESTS|none]   -> LOGDIR/result.raw
# BASE (Track 1): the task's starting point on top of the pinned commit, applied
#   and committed first. TESTS: hidden tests (e.g. those that came with the
#   upstream fix), applied after the patch.
# F: the package's tests pass (with the hidden tests) and the hidden probe's
#    functional check passes.
# R: malloc-guard recorded no allocation in any guarded update() cycle.
PKG=$1; PROBE=$2; PATCH=$3; LOGDIR=$4; BASE=${5:-none}; TESTS=${6:-none}
WS=${RTSWE_WS:-/ws}
REPO=$WS/src/ros2_controllers
CMAKE_ARGS="-G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON"
cd "$WS"
mkdir -p "$LOGDIR"
applied=1; build=1; F=0; R=0; tests=''; pkg_ok=0; func_ok=0; probe_json=null; hidden=1

if [[ "$BASE" != none ]]; then
  { git -C "$REPO" apply --whitespace=nowarn "$BASE" && git -C "$REPO" add -A &&
    git -C "$REPO" -c user.name=rtswe -c user.email=rtswe@localhost commit -qm "rtswe: task base"; } \
    > "$LOGDIR/apply_base.log" 2>&1 || { echo "task base did not apply" >&2; exit 3; }
fi
if [[ "$PATCH" != none ]]; then
  git -C "$REPO" apply --whitespace=nowarn "$PATCH" > "$LOGDIR/apply.log" 2>&1 || applied=0
fi
if [[ $applied == 1 && "$TESTS" != none ]]; then
  git -C "$REPO" apply --whitespace=nowarn "$TESTS" > "$LOGDIR/apply_tests.log" 2>&1 || hidden=0
fi
echo "$PKG" >> "$WS/.rtswe_dirty"   # native backend: rebuild on the next reset

if [[ $applied == 1 ]]; then
  colcon build --packages-select "$PKG" --cmake-args $CMAKE_ARGS > "$LOGDIR/build_pkg.log" 2>&1 || build=0
else
  build=0
fi
if [[ $build == 1 ]]; then
  colcon test --packages-select "$PKG" > "$LOGDIR/test_pkg.log" 2>&1 || true
  colcon test-result --test-result-base "build/$PKG" > "$LOGDIR/test_result.log" 2>&1 && pkg_ok=1
  tests=$(tail -1 "$LOGDIR/test_result.log")
  # Configure the probe from scratch against this workspace. In an overlay workspace
  # (/ws_N) a CMake cache from an earlier run can still point at the underlay's
  # unmodified headers while the test loads this workspace's modified library: a
  # patch that changes a class layout then crashes the probe (ABI mismatch).
  rm -rf "$WS/build/rtswe_eval" "$WS/install/rtswe_eval"
  colcon build --packages-select rtswe_eval --cmake-args $CMAKE_ARGS -DRTSWE_ONLY="$PROBE" \
    -DROS2_CONTROLLERS_SRC="$REPO" > "$LOGDIR/build_probe.log" 2>&1 || build=0
fi
if [[ $build == 1 ]]; then
  colcon test --packages-select rtswe_eval --ctest-args -R "^${PROBE}\$" > /dev/null 2>&1 || true
  cp "$WS/log/latest_test/rtswe_eval/stdout_stderr.log" "$LOGDIR/probe.log" 2>/dev/null || true
  probe_json=$(sed -n 's/^[0-9]*: \[rtswe-json\] //p' "$LOGDIR/probe.log" | head -1)
  func_ok=$(sed -n 's/^[0-9]*: \[rtswe\] functional_ok=\([01]\).*/\1/p' "$LOGDIR/probe.log" | head -1)
  [[ -z "$probe_json" ]] && probe_json=null
  [[ $pkg_ok == 1 && "$func_ok" == 1 && $hidden == 1 ]] && F=1
  if [[ "$probe_json" != null ]] && grep -q '"allocating_cycles":0,' <<< "$probe_json"; then R=1; fi
fi
printf '{"applied":%s,"build":%s,"hidden_tests":%s,"F":%s,"R":%s,"tests":"%s","alloc":%s}\n' \
  "$applied" "$build" "$hidden" "$F" "$R" "$tests" "$probe_json" > "$LOGDIR/result.raw"
