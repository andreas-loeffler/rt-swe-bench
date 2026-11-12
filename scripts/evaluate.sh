#!/usr/bin/env bash
# Score one patch on one task.
#   scripts/evaluate.sh tasks/rt-001 path/to/patch.diff   # or "none" for the unpatched code
# Backends (RTSWE_BACKEND):
#   docker (default)  fresh container from the pinned image per call
#   native            a host workspace from scripts/setup_native.sh ($RTSWE_WS,
#                     default /ws), reset to the pinned state first; one
#                     evaluation per workspace at a time
# Prints one JSON line:
#   {"task":..,"applied":..,"build":..,"F":0|1,"R":0|1,"tests":"..","alloc":{...},"logs":..}
set -euo pipefail
cd "$(dirname "$0")/.."

TASK_DIR="${1:?task dir}"
PATCH="${2:?patch file or none}"
BACKEND="${RTSWE_BACKEND:-docker}"
IMAGE="${IMAGE:-rt-swe-bench/base:jazzy-d71703b}"
LOG_DIR="${LOG_DIR:-results/raw/$(basename "$TASK_DIR")/$(date +%Y%m%d-%H%M%S)-$$}"
mkdir -p "$LOG_DIR"
LOG_DIR=$(realpath "$LOG_DIR")

read -r TASK_ID PKG PROBE BASE TESTS < <(python3 -c '
import json, sys
t = json.load(open(sys.argv[1] + "/task.json"))
print(t["id"], t["package"], t["probe"], t.get("base_patch", "none"), t.get("test_patch", "none"))' "$TASK_DIR")
# Optional task files (Track 1): absolute paths, or none.
[[ "$BASE" != none ]] && BASE=$(realpath "$TASK_DIR/$BASE")
[[ "$TESTS" != none ]] && TESTS=$(realpath "$TASK_DIR/$TESTS")

# Edits to the package's own test files are dropped before scoring (as in SWE-bench): an agent must not
# make the functional check pass by changing what the existing tests assert.
if [[ "$PATCH" != none ]]; then
  python3 - "$PATCH" "$LOG_DIR/patch_scored.diff" "$LOG_DIR/dropped_test_files.txt" <<'PYEOF'
import re, sys
src, dst, dropped = sys.argv[1:4]
blocks = re.split(r"(?m)^(?=diff --git )", open(src, errors="replace").read())
keep, drop = [], []
for b in blocks:
    m = re.match(r"diff --git a/(\S+)", b)
    (drop if m and re.search(r"(^|/)tests?/", m.group(1)) else keep).append(m.group(1) if m and re.search(r"(^|/)tests?/", m.group(1)) else b)
open(dst, "w").write("".join(keep))
open(dropped, "w").write("\n".join(drop))
PYEOF
  PATCH="$LOG_DIR/patch_scored.diff"
fi

if [[ "$BACKEND" == docker ]]; then
  PATCH_MOUNT=(); PATCH_IN=none; BASE_IN=none; TESTS_IN=none
  if [[ "$PATCH" != none ]]; then
    PATCH_MOUNT+=(-v "$(realpath "$PATCH"):/rtswe/patch.diff:ro"); PATCH_IN=/rtswe/patch.diff
  fi
  if [[ "$BASE" != none ]]; then PATCH_MOUNT+=(-v "$BASE:/rtswe/base.diff:ro"); BASE_IN=/rtswe/base.diff; fi
  if [[ "$TESTS" != none ]]; then PATCH_MOUNT+=(-v "$TESTS:/rtswe/tests.diff:ro"); TESTS_IN=/rtswe/tests.diff; fi
  # no network: parallel containers on the default bridge discover each other's ROS 2
  # nodes and fail each other's package tests
  docker run --rm --network "${RTSWE_DOCKER_NET:-none}" \
    -v "$PWD/harness/rtswe_eval:/ws/src/rtswe_eval:ro" \
    -v "$PWD/scripts/score_in_ws.sh:/rtswe/score_in_ws.sh:ro" \
    -v "$LOG_DIR:/rtswe/logs" \
    "${PATCH_MOUNT[@]}" \
    "$IMAGE" bash -c "bash /rtswe/score_in_ws.sh $PKG $PROBE $PATCH_IN /rtswe/logs $BASE_IN $TESTS_IN; \
                      chown -R $(id -u):$(id -g) /rtswe/logs" > /dev/null
elif [[ "$BACKEND" == native ]]; then
  PATCH_IN=none
  if [[ "$PATCH" != none ]]; then PATCH_IN=$(realpath "$PATCH"); fi
  WS=${RTSWE_WS:-/ws}
  rm -rf "$WS/src/rtswe_eval" && cp -r harness/rtswe_eval "$WS/src/rtswe_eval"
  bash scripts/native_reset.sh > "$LOG_DIR/reset.log" 2>&1
  bash -c "source /opt/rtswe/env.sh && bash scripts/score_in_ws.sh $PKG $PROBE $PATCH_IN $LOG_DIR $BASE $TESTS"
else
  echo "unknown RTSWE_BACKEND=$BACKEND" >&2; exit 2
fi

python3 - "$TASK_ID" "$LOG_DIR" <<'EOF'
import json, sys
task, log_dir = sys.argv[1], sys.argv[2]
r = json.loads(open(f"{log_dir}/result.raw").read().strip().splitlines()[-1])
print(json.dumps({"task": task, **r, "logs": log_dir}))
EOF
