// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-010 (joint_trajectory_controller, Track 2). Sampling the
// trajectory resets the output point by assigning a fresh JointTrajectoryPoint,
// which frees its vectors; refilling them allocates on every sample, twice per
// update().
//
// Upstream fixture: 3 joints, position command, position and velocity state.
// A 3-point trajectory with positions and velocities (0.5 s apart) arrives
// over the topic. Warm-up: three unguarded cycles, in which the controller adopts
// the trajectory. Then 200 guarded cycles at 100 Hz cover the execution and the
// hold at the last point. The commanded positions are recorded outside the
// guard every 25 cycles and compared with the unpatched upstream code.

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
constexpr int kCycles = 200;
constexpr int kEvery = 25;               // record the commands every 25 cycles
constexpr int kSamples = kCycles / kEvery;
// From the unpatched upstream code: joint1..3 command positions at cycles
// 0, 25, ..., 175 (before each of those cycles) and after the last cycle.
constexpr double kExpected[(kSamples + 1) * 3] = {
  1.1219636000000002, 2.1230004, 3.1240372000000001,
  2.3625536, 3.4215104000000003, 4.4804671999999997,
  3.3581963999999997, 4.4581964000000003, 5.5581963999999999,
  5.8936064000000004, 6.9936064, 8.0936064000000023,
  7.7354863999999992, 8.8345532800000015, 9.933620160000002,
  9.1366463999999983, 10.183585279999999, 11.23052416,
  10.1, 11.109999999999999, 12.119999999999999,
  10.1, 11.109999999999999, 12.119999999999999,
  10.1, 11.109999999999999, 12.119999999999999};
}  // namespace

class JtcSampleTask : public TrajectoryControllerTest
{
};

TEST_F(JtcSampleTask, UpdateDoesNotAllocate)
{
  rclcpp::executors::MultiThreadedExecutor executor;
  SetUpAndActivateTrajectoryController(executor);

  builtin_interfaces::msg::Duration time_from_start;
  time_from_start.sec = 0;
  time_from_start.nanosec = 500000000;
  const std::vector<std::vector<double>> points{
    {{3.3, 4.4, 5.5}}, {{7.7, 8.8, 9.9}}, {{10.10, 11.11, 12.12}}};
  const std::vector<std::vector<double>> velocities{
    {{0.5, 0.5, 0.5}}, {{0.4, 0.4, 0.4}}, {{0.0, 0.0, 0.0}}};
  publish(time_from_start, points, rclcpp::Time(), {}, velocities);
  traj_controller_->wait_for_trajectory(executor);

  const auto period = rclcpp::Duration::from_seconds(0.01);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return traj_controller_->update(t, p);},
    rclcpp::Time(0, 0, RCL_STEADY_TIME), period);
  ASSERT_TRUE(start.has_value());

  std::array<double, (kSamples + 1) * 3> got{};
  const auto r = rtswe_eval::run_probe(
    "joint_trajectory_controller", "rt-010: positions+velocities trajectory, warm-up 3", kCycles,
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
