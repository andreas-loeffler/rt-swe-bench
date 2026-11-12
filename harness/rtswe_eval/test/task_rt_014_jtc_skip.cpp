// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-014 (joint_trajectory_controller, Track 1; reverted
// upstream fix ros2_controllers#2359). The last point of a trajectory has only
// velocities (allow_integration_in_goal_trajectories), and the last segment is
// shorter than the control period. Sampling jumps past it, so the last point's
// positions are never deduced while that segment runs; the unpatched controller
// then samples an empty positions vector (upstream: segfault). The merged fix
// deduces them in sample(), which grows the empty vector inside update().
//
// Upstream fixture: 3 joints, position command, position and velocity state.
// Points at 0.100 s and 0.203 s with positions and velocities, the last point
// at 0.207 s with velocities only. Warm-up: three unguarded cycles, in which the
// controller adopts the trajectory. Then 50 guarded cycles at 100 Hz; the
// samples at 0.21 s and later are past the 4 ms last segment. The commanded
// positions are recorded outside the guard every 5 cycles and after the last
// one, and compared with values from the fixed code.

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <thread>
#include <vector>

#include "rtswe_eval/probe.hpp"
#include "test_trajectory_controller_utils.hpp"  // upstream fixture

using test_trajectory_controllers::TrajectoryControllerTest;

namespace
{
constexpr int kCycles = 50;
constexpr int kEvery = 5;
constexpr int kSamples = kCycles / kEvery;
// From the fixed code (merged upstream fix): joint1..3 command positions before
// cycles 0, 5, ..., 45 and after the last cycle.
constexpr double kExpected[(kSamples + 1) * 3] = {
  1.1153000000000002, 2.1153, 3.1153000000000004,
  1.1767999999999998, 2.1767999999999996, 3.1768000000000005,
  1.229384750262417, 2.2293847502624167, 3.2293847502624162,
  1.2773819627409229, 2.2773819627409226, 3.2773819627409213,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996,
  1.302, 2.3019999999999996, 3.3019999999999996};

builtin_interfaces::msg::Duration at(double s)
{
  builtin_interfaces::msg::Duration d;
  d.sec = static_cast<int32_t>(s);
  d.nanosec = static_cast<uint32_t>(std::lround((s - d.sec) * 1e9));
  return d;
}
}  // namespace

class JtcSkipTask : public TrajectoryControllerTest
{
protected:
  void publish_velocity_tail()
  {
    const auto topic = trajectory_publisher_->get_topic_name();
    for (int i = 0; i < 20 && node_->count_subscribers(topic) == 0; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    trajectory_msgs::msg::JointTrajectory msg;
    msg.joint_names = {joint_names_[0], joint_names_[1], joint_names_[2]};
    msg.header.stamp = rclcpp::Time();  // start at the first sample
    msg.points.resize(3);
    msg.points[0].time_from_start = at(0.100);
    msg.points[0].positions = {1.2, 2.2, 3.2};
    msg.points[0].velocities = {1.0, 1.0, 1.0};
    msg.points[1].time_from_start = at(0.203);
    msg.points[1].positions = {1.3, 2.3, 3.3};
    msg.points[1].velocities = {1.0, 1.0, 1.0};
    msg.points[2].time_from_start = at(0.207);  // velocities only
    msg.points[2].velocities = {0.0, 0.0, 0.0};
    trajectory_publisher_->publish(msg);
  }
};

TEST_F(JtcSkipTask, UpdateDoesNotAllocate)
{
  rclcpp::executors::MultiThreadedExecutor executor;
  SetUpAndActivateTrajectoryController(
    executor, {rclcpp::Parameter("allow_integration_in_goal_trajectories", true)});

  publish_velocity_tail();
  traj_controller_->wait_for_trajectory(executor);

  const auto period = rclcpp::Duration::from_seconds(0.01);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return traj_controller_->update(t, p);},
    rclcpp::Time(0, 0, RCL_STEADY_TIME), period);
  ASSERT_TRUE(start.has_value());

  std::array<double, (kSamples + 1) * 3> got{};
  const auto r = rtswe_eval::run_probe(
    "joint_trajectory_controller", "rt-014: velocity-only last point, skipped last segment, warm-up 3",
    kCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return traj_controller_->update(t, p);},
    0.01, *start,
    [&](int k, const rclcpp::Time &) {
      if (k % kEvery == 0)
      {
        for (std::size_t j = 0; j < 3; ++j)
        {
          got[static_cast<std::size_t>(k / kEvery) * 3 + j] =
            pos_cmd_interfaces_[j]->get_optional().value_or(std::nan(""));
        }
      }
    });
  rtswe_eval::print(r);
  for (std::size_t j = 0; j < 3; ++j)
  {
    got[kSamples * 3 + j] = pos_cmd_interfaces_[j]->get_optional().value_or(std::nan(""));
  }

  bool match = true;
  std::cout << "[rtswe] values=" << std::setprecision(17);
  for (std::size_t i = 0; i < got.size(); ++i)
  {
    std::cout << got[i] << (i + 1 < got.size() ? "," : "\n");
    match = match && std::abs(got[i] - kExpected[i]) < 1e-9;
  }
  rtswe_eval::report_functional(r.failed_cycles == 0 && match);
  EXPECT_TRUE(match);
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
