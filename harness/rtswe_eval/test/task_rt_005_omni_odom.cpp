// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-005 (omni_wheel_drive_controller, Track 2, odometry).
// Upstream fixture, 4 wheels, closed loop on wheel velocity feedback, odometry
// TF disabled. The wheels report unequal speeds, so the odometry solves a real
// least-squares problem every cycle; upstream allocates for the feedback vector
// and for the dynamic-size Eigen matrices and SVD in the odometry.
//
// A twist is stored, as the subscriber would, then three unguarded update()
// calls (first odometry publish), then 50 guarded cycles.

#include <cmath>
#include <iomanip>
#include <limits>

#include "rtswe_eval/probe_main.hpp"
#include "test_omni_wheel_drive_controller.hpp"  // upstream fixture

namespace
{
class TaskOmni : public TestableOmniWheelDriveController
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { received_velocity_msg_.set(m); }
  double x() const { return odometry_.getX(); }
  double y() const { return odometry_.getY(); }
  double heading() const { return odometry_.getHeading(); }
  double vx() const { return odometry_.getLinearXVel(); }
  double vy() const { return odometry_.getLinearYVel(); }
  double wz() const { return odometry_.getAngularVel(); }
};

// From the unpatched upstream code: 4 wheel commands, then odometry x, y,
// heading and the estimated body velocities vx, vy, wz.
constexpr double kExpected[10] = {
  -5.9999999999999991, 9, 4.0000000000000009, -10.999999999999998,
  -0.0051318179611828376, 0.0052670103450877769, -0.026000000000000002,
  -0.099999999999999964, 0.10000000000000002, -0.50000000000000011};
}  // namespace

class OmniOdomTask : public OmniWheelDriveControllerFixture<TaskOmni>
{
};

TEST_F(OmniOdomTask, UpdateDoesNotAllocate)
{
  ASSERT_EQ(
    InitController(
      wheel_names_, 0.0,
      {rclcpp::Parameter("position_feedback", false), rclcpp::Parameter("enable_odom_tf", false)}),
    controller_interface::return_type::OK);
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->export_reference_interfaces();
  wheels_vel_states_ = {1.0, 2.0, 3.0, 4.0};
  assignResourcesVelFeedback();
  ASSERT_TRUE(activate_succeeds(controller_));

  const auto period = rclcpp::Duration::from_seconds(0.001);
  const rclcpp::Time t0(0, 0, RCL_ROS_TIME);
  // The twist goes in before the warm-up: without a reference the controller
  // returns before the odometry, and nothing would be published yet.
  controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.5, 0.2));
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    t0, period);
  ASSERT_TRUE(start.has_value());

  const auto r = rtswe_eval::run_probe(
    "omni_wheel_drive_controller", "rt-005: 4 wheels, velocity feedback, TF off, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.001, *start);
  rtswe_eval::print(r);

  double got[10];
  for (std::size_t i = 0; i < 4; ++i)
  {
    got[i] = command_itfs_[i]->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
  }
  got[4] = controller_->x();
  got[5] = controller_->y();
  got[6] = controller_->heading();
  got[7] = controller_->vx();
  got[8] = controller_->vy();
  got[9] = controller_->wz();
  bool match = command_itfs_.size() == 4;
  std::cout << "[rtswe] values=" << std::setprecision(17);
  for (std::size_t i = 0; i < 10; ++i)
  {
    std::cout << got[i] << (i < 9 ? "," : "\n");
    match = match && std::abs(got[i] - kExpected[i]) < 1e-9;
  }
  rtswe_eval::report_functional(r.failed_cycles == 0 && match);
  EXPECT_TRUE(match);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
