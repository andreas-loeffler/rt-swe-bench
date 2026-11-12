#!/usr/bin/env bash
# Check that a task discriminates: score the unpatched code, the reference
# patch and the naive patch, and compare with what the task's track requires.
#   scripts/validate_task.sh tasks/rt-001
#
#               Track 1 (bug fix)   Track 2 (RT remediation)
#   unpatched   F=0                 F=1, R=0
#   reference   F=1, R=1            F=1, R=1
#   naive       F=1, R=0            F=1, R=0
set -euo pipefail
cd "$(dirname "$0")/.."

TASK_DIR="${1:?task dir}"
TRACK=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]+"/task.json"))["track"])' "$TASK_DIR")
OUT="${VALIDATION_OUT:-$TASK_DIR/validation.jsonl}"  # verify.sh writes elsewhere
: > "$OUT"

ok=1
check() {  # variant patch expected_F expected_R("-" = any)
  local variant=$1 patch=$2 ef=$3 er=$4
  local res f r
  res=$(scripts/evaluate.sh "$TASK_DIR" "$patch")
  echo "$res" | python3 -c 'import json,sys; r=json.load(sys.stdin); r["variant"]=sys.argv[1]; print(json.dumps(r))' "$variant" >> "$OUT"
  f=$(echo "$res" | python3 -c 'import json,sys; print(json.load(sys.stdin)["F"])')
  r=$(echo "$res" | python3 -c 'import json,sys; print(json.load(sys.stdin)["R"])')
  local verdict=PASS
  if [[ "$f" != "$ef" || ( "$er" != "-" && "$r" != "$er" ) ]]; then verdict=FAIL; ok=0; fi
  printf '%-10s F=%s R=%s   expected F=%s R=%s   %s\n' "$variant" "$f" "$r" "$ef" "$er" "$verdict"
}

if [[ "$TRACK" == 1 ]]; then
  check unpatched none 0 -
else
  check unpatched none 1 0
fi
check reference "$TASK_DIR/reference.diff" 1 1
check naive "$TASK_DIR/naive.diff" 1 0

if [[ $ok == 1 ]]; then echo "$(basename "$TASK_DIR"): valid"; else echo "$(basename "$TASK_DIR"): INVALID"; exit 1; fi
