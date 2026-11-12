// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-003 (steering_controllers_library, Track 2). Upstream
// testable subclass (Ackermann kinematics) and parameter file, closed loop.
//
// Warm-up: three unguarded update() calls after activation. Their one-time allocations
// (first odometry and state publish) are outside this task. Then a twist is
// stored, as the subscriber would, and 50 cycles are guarded.

#include <cmath>
#include <iomanip>
#include <limits>
#include <string>

#include "rtswe_eval/probe_main.hpp"
#include "test_steering_controllers_library.hpp"  // upstream fixture

namespace
{
class TaskSteering : public TestableSteeringControllersLibrary
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { input_ref_.set(m); }
};

// Command interface values after the guarded loop, from the unpatched upstream
// code (traction right, traction left, steering right, steering left).
constexpr double kExpected[4] = {
  2.2222222222222223, 2.2222222222222223, 0.57587453049636572, 0.57587453049636572};
}  // namespace

class SteeringTask : public SteeringControllersLibraryFixture<TaskSteering>
{
};

TEST_F(SteeringTask, UpdateDoesNotAllocate)
{
  SetUpController();
  ASSERT_TRUE(configure_succeeds(controller_));
  ASSERT_TRUE(activate_succeeds(controller_));

  const auto period = rclcpp::Duration::from_seconds(0.001);
  const rclcpp::Time t0(0, 0, RCL_ROS_TIME);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    t0, period);
  ASSERT_TRUE(start.has_value());

  controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.0, 0.2));
  const auto r = rtswe_eval::run_probe(
    "steering_controllers_library", "rt-003: ackermann, closed loop, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.001, *start);
  rtswe_eval::print(r);

  bool match = command_itfs_.size() == 4;
  std::cout << "[rtswe] commands=" << std::setprecision(17);
  for (std::size_t i = 0; i < command_itfs_.size() && i < 4; ++i)
  {
    const double v =
      command_itfs_[i]->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
    std::cout << v << (i + 1 < command_itfs_.size() ? "," : "\n");
    match = match && std::abs(v - kExpected[i]) < 1e-9;
  }
  rtswe_eval::report_functional(r.failed_cycles == 0 && match);
  EXPECT_TRUE(match);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
