#!/usr/bin/env bash
# Native backend: create an extra workspace /ws_N for a parallel worker. It
# holds its own git checkout of ros2_controllers at the pinned commit and
# overlays the fully built /ws, so it only builds the packages a run changes.
#   scripts/add_native_ws.sh 2      # -> /ws_2, then run with RTSWE_WS=/ws_2
set -euo pipefail
N="${1:?workspace number}"
WS=/ws_$N
PINNED=$(sed -n 's/^ros2_controllers=//p' /opt/rtswe/source-manifest.txt)
mkdir -p "$WS/src"
rm -rf "$WS/src/ros2_controllers"
git clone --quiet --no-hardlinks /ws/src/ros2_controllers "$WS/src/ros2_controllers"
git -C "$WS/src/ros2_controllers" checkout --quiet "$PINNED"
echo "$WS ready (overlay on /ws); use RTSWE_WS=$WS"
