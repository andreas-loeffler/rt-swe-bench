// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-012 (steering_controllers_library, Track 1). Task base:
// the fix of ros2_controllers#2083 reverted (non-finite references reach the
// open-loop odometry; the reference check tested linear.y instead of
// angular.z), on top of rt-003's in-place get_commands().
//
// Upstream testable subclass (Ackermann) and parameters, open loop. Warm-up:
// three unguarded cycles with a valid twist. Then 50 guarded cycles with a
// twist whose angular.z is +Inf, stored fresh before every cycle. Commands and
// odometry must stay finite and match the fixed behavior, without allocating.

#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <thread>

#include "rtswe_eval/probe_main.hpp"
#include "test_steering_controllers_library.hpp"  // upstream fixture

namespace
{
class TaskSteering : public TestableSteeringControllersLibrary
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { input_ref_.set(m); }
  double x() const { return odometry_.get_x(); }
  double y() const { return odometry_.get_y(); }
  double heading() const { return odometry_.get_heading(); }
};

// From the fixed upstream code: odometry x, y, heading after the guarded cycles.
constexpr double kExpected[3] = {0.011924999999999996, 0, 0};
}  // namespace

class SteeringNanTask : public SteeringControllersLibraryFixture<TaskSteering>
{
};

TEST_F(SteeringNanTask, NonFiniteReferenceKeepsOdometryFinite)
{
  auto node_options = controller_->define_custom_node_options();
  node_options.append_parameter_override("open_loop", true);
  SetUpController("test_steering_controllers_library", node_options);
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->set_chained_mode(false);
  ASSERT_TRUE(activate_succeeds(controller_));

  const auto period = rclcpp::Duration::from_seconds(0.001);
  rclcpp::Time t(0, 0, RCL_ROS_TIME);
  for (int k = 0; k < 3; ++k)
  {
    controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.0, 0.2, t));
    ASSERT_EQ(controller_->update(t, period), controller_interface::return_type::OK);
    t = t + period;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  const double inf = std::numeric_limits<double>::infinity();
  const auto r = rtswe_eval::run_probe(
    "steering_controllers_library", "rt-012: open loop, angular.z = +Inf, warm-up 3", 50,
    [&](const rclcpp::Time & tt, const rclcpp::Duration & p) {return controller_->update(tt, p);},
    0.001, t,
    [&](int, const rclcpp::Time & tt) {
      controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.0, inf, tt));
    });
  rtswe_eval::print(r);

  bool finite = command_itfs_.size() == 4;
  for (const auto & itf : command_itfs_)
  {
    finite = finite && std::isfinite(itf->get_optional().value_or(std::nan("")));
  }
  const double got[3] = {controller_->x(), controller_->y(), controller_->heading()};
  bool match = true;
  std::cout << "[rtswe] values=" << std::setprecision(17);
  for (std::size_t i = 0; i < 3; ++i)
  {
    std::cout << got[i] << (i < 2 ? "," : "\n");
    match = match && std::isfinite(got[i]) && std::abs(got[i] - kExpected[i]) < 1e-9;
  }
  std::cout << "[rtswe] commands_finite=" << finite << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && finite && match);
  EXPECT_TRUE(finite);
  EXPECT_TRUE(match);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
