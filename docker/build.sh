#!/usr/bin/env bash
# Build the pinned RT-SWE-Bench base image.
#   docker/build.sh                 # default tag
#   BUILD_JOBS=4 docker/build.sh    # less RAM (e.g. the 32 GB PC)
set -euo pipefail
cd "$(dirname "$0")"

TAG="${TAG:-rt-swe-bench/base:jazzy-d71703b}"

DOCKER_BUILDKIT=1 docker build \
  --build-arg BUILD_JOBS="${BUILD_JOBS:-6}" \
  --build-arg PARALLEL_WORKERS="${PARALLEL_WORKERS:-3}" \
  -t "${TAG}" .

docker image inspect --format '{{.Id}}' "${TAG}"
