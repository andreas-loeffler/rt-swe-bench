#!/usr/bin/env bash
# Validate synthetic kernels: the reference passes both checks, every allocating
# variant passes the functional check but allocates, the stub fails the
# functional check. Runs in the benchmark image:
#   synthetic/validate.sh                  # all kernels
#   synthetic/validate.sh moving_average   # one kernel
set -u
cd "$(dirname "$0")/.."
IMAGE="${IMAGE:-rt-swe-bench/base:jazzy-d71703b}"
kernels=("$@")
[[ ${#kernels[@]} -eq 0 ]] && mapfile -t kernels < <(ls synthetic/fixtures)
docker run --rm -v "$PWD":/rtswe:ro "$IMAGE" bash -c '
  cd /rtswe; ok_all=1
  for k in '"${kernels[*]}"'; do
    d=synthetic/fixtures/$k
    check() {  # label file expected_F expected_R("-" = any)
      local res f r v
      res=$(bash synthetic/score.sh "$d" "$2"); f=$(sed -n "s/.*\"F\":\([01]\).*/\1/p" <<< "$res"); r=$(sed -n "s/.*\"R\":\([01]\).*/\1/p" <<< "$res")
      v=PASS; if [[ "$f" != "$3" || ( "$4" != - && "$r" != "$4" ) ]]; then v=FAIL; ok_all=0; fi
      printf "  %-28s F=%s R=%s  expected F=%s R=%s  %s\n" "$1" "$f" "$r" "$3" "$4" "$v"
      [[ $v == FAIL ]] && echo "    $res"
    }
    echo "== $k"
    check reference "$d/reference.hpp" 1 1
    for v in "$d"/variants/*.hpp; do check "variant $(basename "$v" .hpp)" "$v" 1 0; done
    check stub "$d/stub.hpp" 0 -
  done
  [[ $ok_all == 1 ]] && echo "all valid" || { echo "INVALID kernels above"; exit 1; }'
