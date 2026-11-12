#!/usr/bin/env bash
# Native backend only: put the workspace ($RTSWE_WS, default /ws) back to the pinned state. Resets the
# ros2_controllers checkout to the pinned commit (dropping a committed task
# base and any edits) and rebuilds each package that was changed
# or built from changed sources since the last reset ($WS/.rtswe_dirty).
set -euo pipefail
source /opt/rtswe/env.sh
WS=${RTSWE_WS:-/ws}
cd "$WS/src/ros2_controllers"
PINNED=$(sed -n 's/^ros2_controllers=//p' /opt/rtswe/source-manifest.txt)
pkgs=$( { git diff --name-only "$PINNED"; git ls-files --others --exclude-standard; } | cut -d/ -f1)
pkgs="$pkgs $(cat "$WS/.rtswe_dirty" 2>/dev/null || true)"
git reset -q --hard "$PINNED"
git clean -fdq
valid=""
for p in $(echo "$pkgs" | tr ' ' '\n' | sort -u); do
  [[ -n "$p" && -f "$p/package.xml" ]] && valid="$valid $p"
done
if [[ -n "$valid" ]]; then
  cd "$WS"
  colcon build --packages-select $valid --cmake-args -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_TESTING=ON > /tmp/rtswe_reset_build.log 2>&1 || { tail -40 /tmp/rtswe_reset_build.log; exit 1; }
fi
rm -f "$WS/.rtswe_dirty"
echo "reset:${valid:- nothing to rebuild}"
