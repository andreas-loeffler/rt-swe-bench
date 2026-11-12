# RT-SWE-Bench base image: ROS 2 Jazzy + ros2_controllers + malloc-guard.
# Every input is pinned (image digest, git commits); installed apt versions are
# recorded in /opt/rtswe/apt-manifest.txt. Build with docker/build.sh.
#
# Clones use HTTPS on purpose: the repos are public and the build has no SSH keys.
FROM ros:jazzy-ros-base@sha256:8eed0a4669bbf43ee331878a4c39bd8ebddea8e3312a58b4fae8edb14526b617

ARG ROS2_CONTROLLERS_SHA=d71703b362a6bbb4637a246d22b9960f8d149b20
ARG MALLOC_GUARD_SHA=cb7c4459d1f12527c542c65433de6578df83d749
# Compile jobs per package x packages in parallel. 6 x 3 stays under ~40 GB RAM
# for the heaviest translation units (joint_trajectory_controller tests).
ARG BUILD_JOBS=6
ARG PARALLEL_WORKERS=3

ENV DEBIAN_FRONTEND=noninteractive \
    CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=5G \
    WS=/ws

RUN apt-get update && apt-get install -y --no-install-recommends \
      binutils ccache cmake git ninja-build ripgrep \
      python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# --- malloc-guard (C++20, GOT/PLT hooking via plthook) ------------------------
RUN git clone --quiet https://github.com/intrinsic-ai/malloc-guard.git /opt/src/malloc-guard \
 && git -C /opt/src/malloc-guard checkout --quiet ${MALLOC_GUARD_SHA} \
 && cmake -S /opt/src/malloc-guard -B /opt/src/malloc-guard/build -G Ninja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX=/opt/malloc_guard \
 && cmake --build /opt/src/malloc-guard/build \
 && cmake --install /opt/src/malloc-guard/build
ENV CMAKE_PREFIX_PATH=/opt/malloc_guard \
    LD_LIBRARY_PATH=/opt/malloc_guard/lib

# --- ros2_controllers at the pinned commit ------------------------------------
WORKDIR /ws
RUN git clone --quiet https://github.com/ros-controls/ros2_controllers.git src/ros2_controllers \
 && git -C src/ros2_controllers checkout --quiet ${ROS2_CONTROLLERS_SHA}

RUN apt-get update \
 && rosdep update --rosdistro jazzy \
 && rosdep install --from-paths src --ignore-src -y --rosdistro jazzy \
 && rm -rf /var/lib/apt/lists/*

# Full build with tests: leaves build/ and a warm ccache in the image, so an
# agent's edit only recompiles the touched translation units.
RUN . /opt/ros/jazzy/setup.sh \
 && MAKEFLAGS=-j${BUILD_JOBS} colcon build \
      --parallel-workers ${PARALLEL_WORKERS} \
      --event-handlers console_direct- console_stderr- status- \
      --cmake-args -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_TESTING=ON \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
 && ccache --show-stats

# Condition C3: clang compiles a package to LLVM IR for the static call tree
# (harness/calltree); llvm-18 provides llvm-cxxfilt for clang's newer manglings.
# Its own layer after the build, so adding it does not invalidate the build cache.
RUN apt-get update && apt-get install -y --no-install-recommends clang-18 llvm-18 \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/rtswe \
 && dpkg-query -W -f='${Package}=${Version}\n' > /opt/rtswe/apt-manifest.txt \
 && printf 'ros2_controllers=%s\nmalloc_guard=%s\n' "${ROS2_CONTROLLERS_SHA}" "${MALLOC_GUARD_SHA}" \
      > /opt/rtswe/source-manifest.txt

COPY rtswe_entrypoint.sh /rtswe_entrypoint.sh
ENTRYPOINT ["/rtswe_entrypoint.sh"]
CMD ["bash"]
