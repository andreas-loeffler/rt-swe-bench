// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-004 (omni_wheel_drive_controller, Track 2, odometry TF).
// Upstream fixture, 4 wheels, open loop (odometry integrates the reference),
// odometry TF enabled. In this configuration the only per-cycle allocation
// upstream is the TF message built inside update().
//
// A twist is stored, as the subscriber would, then three unguarded update()
// calls (first publish of the odometry and TF messages), then 50 guarded cycles.

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
  const geometry_msgs::msg::TransformStamped & transform() const { return transform_; }
  const tf2_msgs::msg::TFMessage & last_tf() const
  {
    return realtime_odometry_transform_publisher_->msg_;
  }
};

// From the unpatched upstream code: 4 wheel commands, then odometry x, y, heading.
constexpr double kExpected[7] = {
  -5.9999999999999991, 9, 4.0000000000000009, -10.999999999999998,
  0.051863863837001131, 0.026269928872004598, 0.010400000000000008};
}  // namespace

class OmniTfTask : public OmniWheelDriveControllerFixture<TaskOmni>
{
};

TEST_F(OmniTfTask, UpdateDoesNotAllocate)
{
  ASSERT_EQ(
    InitController(wheel_names_, 0.0, {rclcpp::Parameter("open_loop", true)}),
    controller_interface::return_type::OK);
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->export_reference_interfaces();
  assignResourcesPosFeedback();
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
    "omni_wheel_drive_controller", "rt-004: 4 wheels, open loop, odom TF on, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.001, *start);
  rtswe_eval::print(r);

  double got[7];
  for (std::size_t i = 0; i < 4; ++i)
  {
    got[i] = command_itfs_[i]->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
  }
  got[4] = controller_->x();
  got[5] = controller_->y();
  got[6] = controller_->heading();
  bool match = command_itfs_.size() == 4;
  std::cout << "[rtswe] values=" << std::setprecision(17);
  for (std::size_t i = 0; i < 7; ++i)
  {
    std::cout << got[i] << (i < 6 ? "," : "\n");
    match = match && std::abs(got[i] - kExpected[i]) < 1e-9;
  }
  // The published TF keeps one transform with the configured frames, and its
  // translation is one the odometry actually passed through.
  const auto & tf = controller_->last_tf();
  const bool tf_ok = tf.transforms.size() == 1 &&
                     tf.transforms[0].header.frame_id == controller_->transform().header.frame_id &&
                     tf.transforms[0].child_frame_id == controller_->transform().child_frame_id &&
                     tf.transforms[0].transform.translation.x > 0.0 &&
                     tf.transforms[0].transform.translation.x <= controller_->x() + 1e-12;
  std::cout << "[rtswe] tf_ok=" << tf_ok << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && match && tf_ok);
  EXPECT_TRUE(match);
  EXPECT_TRUE(tf_ok);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
