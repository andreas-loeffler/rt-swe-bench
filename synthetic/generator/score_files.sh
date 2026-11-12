#!/usr/bin/env bash
# Score several candidate headers against one kernel's test.cpp with synthetic/score.sh, in parallel.
#   synthetic/generator/score_files.sh <kernel_dir> <file>...   -> one JSON line per file: {"file":..., <score.sh>}
# Runs where malloc-guard is in $MALLOC_GUARD (default /opt/malloc_guard: the image or a native setup).
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DIR=$1; shift
one() {
  printf '{"file":"%s",%s\n' "$(basename "$2")" "$(bash "$ROOT/synthetic/score.sh" "$1" "$2" | sed 's/^{//')"
}
export -f one; export ROOT
printf '%s\n' "$@" | xargs -P "${GEN_JOBS:-8}" -I{} bash -c 'one "$0" "$1"' "$DIR" {}
