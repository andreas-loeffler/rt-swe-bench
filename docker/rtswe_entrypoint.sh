#!/usr/bin/env bash
# Source ROS 2 and the prebuilt workspace, then run the given command.
set -e
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
exec "$@"
