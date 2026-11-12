#!/usr/bin/env bash
# Native setup (no Docker) on Ubuntu 24.04, e.g. a Colab VM. Mirrors
# docker/Dockerfile step by step: same ROS distro, same pinned commits, same
# build flags. Installed apt versions go to /opt/rtswe/apt-manifest.txt so they
# can be compared with the image (docker/apt-manifest.txt).
#
# ROS tooling runs under the system Python 3.12 through /opt/rtswe/py/bin, so a
# host whose `python3` points elsewhere (Colab: 3.13) is left untouched.
# Commands for the workspace run after `source /opt/rtswe/env.sh`.
set -euo pipefail

ROS2_CONTROLLERS_SHA=d71703b362a6bbb4637a246d22b9960f8d149b20
MALLOC_GUARD_SHA=cb7c4459d1f12527c542c65433de6578df83d749
BUILD_JOBS="${BUILD_JOBS:-8}"
PARALLEL_WORKERS="${PARALLEL_WORKERS:-3}"
export DEBIAN_FRONTEND=noninteractive
step() { echo "[setup] $(date +%T) $*"; }

step "ROS 2 apt source"
apt-get update -qq
apt-get install -y -qq --no-install-recommends curl ca-certificates software-properties-common > /dev/null
add-apt-repository -y universe > /dev/null
ver=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest \
      | sed -n 's/.*"tag_name": *"\([^"]*\)".*/\1/p')
curl -sSL -o /tmp/ros2-apt-source.deb \
  "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ver}/ros2-apt-source_${ver}.noble_all.deb"
apt-get install -y -qq /tmp/ros2-apt-source.deb > /dev/null
apt-get update -qq

step "ROS 2 Jazzy base and build tools"
apt-get install -y -qq --no-install-recommends \
  ros-jazzy-ros-base python3-rosdep python3-colcon-common-extensions \
  build-essential binutils ccache cmake git ninja-build ripgrep \
  clang-18 llvm-18 > /dev/null   # clang/llvm: static call tree for condition C3

step "Python 3.12 shims and environment"
mkdir -p /opt/rtswe/py/bin
ln -sf /usr/bin/python3.12 /opt/rtswe/py/bin/python3
ln -sf /usr/bin/python3.12 /opt/rtswe/py/bin/python
for tool in colcon rosdep; do
  printf '#!/bin/sh\nexec /usr/bin/python3.12 /usr/bin/%s "$@"\n' "$tool" > "/opt/rtswe/py/bin/$tool"
  chmod +x "/opt/rtswe/py/bin/$tool"
done
cat > /opt/rtswe/env.sh <<'EOF'
# Same environment as the image's entrypoint, plus the Python 3.12 shims.
export PATH=/opt/rtswe/py/bin:/usr/bin:$PATH
export CMAKE_PREFIX_PATH=/opt/malloc_guard${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}
export LD_LIBRARY_PATH=/opt/malloc_guard/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export CCACHE_DIR=/ccache CCACHE_MAXSIZE=5G
export WS=${RTSWE_WS:-/ws}
set +u
source /opt/ros/jazzy/setup.bash
# /ws is the fully built workspace; an extra workspace (/ws_N, see
# scripts/add_native_ws.sh) overlays it and builds only what it changes.
if [ -f /ws/install/setup.bash ]; then source /ws/install/setup.bash; fi
if [ "$WS" != /ws ] && [ -f "$WS/install/setup.bash" ]; then source "$WS/install/setup.bash"; fi
EOF
set +u; source /opt/rtswe/env.sh; set -u

step "malloc-guard @ ${MALLOC_GUARD_SHA:0:7}"
rm -rf /opt/src/malloc-guard
git clone --quiet https://github.com/intrinsic-ai/malloc-guard.git /opt/src/malloc-guard
git -C /opt/src/malloc-guard checkout --quiet "${MALLOC_GUARD_SHA}"
cmake -S /opt/src/malloc-guard -B /opt/src/malloc-guard/build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/opt/malloc_guard > /dev/null
cmake --build /opt/src/malloc-guard/build > /dev/null
cmake --install /opt/src/malloc-guard/build > /dev/null

step "ros2_controllers @ ${ROS2_CONTROLLERS_SHA:0:7}"
mkdir -p /ws/src
rm -rf /ws/src/ros2_controllers
git clone --quiet https://github.com/ros-controls/ros2_controllers.git /ws/src/ros2_controllers
git -C /ws/src/ros2_controllers checkout --quiet "${ROS2_CONTROLLERS_SHA}"

step "rosdep"
[ -f /etc/ros/rosdep/sources.list.d/20-default.list ] || rosdep init > /dev/null
rosdep update --rosdistro jazzy > /dev/null
cd /ws
rosdep install --from-paths src --ignore-src -y --rosdistro jazzy > /dev/null

step "colcon build (jobs ${BUILD_JOBS} x workers ${PARALLEL_WORKERS})"
MAKEFLAGS=-j${BUILD_JOBS} colcon build \
  --parallel-workers "${PARALLEL_WORKERS}" \
  --event-handlers console_direct- console_stderr- status- \
  --cmake-args -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

dpkg-query -W -f='${Package}=${Version}\n' > /opt/rtswe/apt-manifest.txt
printf 'ros2_controllers=%s\nmalloc_guard=%s\n' "${ROS2_CONTROLLERS_SHA}" "${MALLOC_GUARD_SHA}" \
  > /opt/rtswe/source-manifest.txt
step "done"
