// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified JointTrajectoryController with the upstream test
// fixture (3 joints, position command, position and velocity state). A
// 3-point trajectory (0.5 s between points) is sent over the topic before the
// loop; 200 guarded update() cycles at 100 Hz cover its execution and the hold
// at the last point.

#include <vector>

#include "rtswe_eval/probe.hpp"
#include "test_trajectory_controller_utils.hpp"  // upstream fixture

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

using test_trajectory_controllers::TrajectoryControllerTest;

class JtcProbe : public TrajectoryControllerTest
{
};

TEST_F(JtcProbe, Census)
{
  rclcpp::executors::MultiThreadedExecutor executor;
  SetUpAndActivateTrajectoryController(executor);

  builtin_interfaces::msg::Duration time_from_start;
  time_from_start.sec = 0;
  time_from_start.nanosec = 500000000;
  const std::vector<std::vector<double>> points{
    {{3.3, 4.4, 5.5}}, {{7.7, 8.8, 9.9}}, {{10.10, 11.11, 12.12}}};
  publish(time_from_start, points, rclcpp::Time(), {}, {});
  traj_controller_->wait_for_trajectory(executor);

  const auto r = rtswe_eval::run_probe(
    "joint_trajectory_controller", "3 joints, position command, 3-point trajectory", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return traj_controller_->update(t, p);},
    0.01, rclcpp::Time(0, 0, RCL_STEADY_TIME));
  rtswe_eval::print(r);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

// The fixture initializes and shuts down rclcpp itself.
int main(int argc, char ** argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  rtswe_eval::install_hooks();
  return RUN_ALL_TESTS();
}
