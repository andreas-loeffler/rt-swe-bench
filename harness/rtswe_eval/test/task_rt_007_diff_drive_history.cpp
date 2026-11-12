// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-007 (diff_drive_controller, Track 2). The speed limiter
// reads the last two commands from a std::queue; its std::deque takes a new
// block every ~32 pop/push pairs, so update() allocates periodically.
//
// One wheel per side, position feedback, acceleration and jerk limits on (so
// the limiter really uses both stored commands), short frame id. A fresh twist
// is stored before every cycle, as a subscriber streaming commands would.
// Warm-up: three unguarded cycles. Then 200 guarded cycles.

#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "diff_drive_controller/diff_drive_controller.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rtswe_eval/probe_main.hpp"

using hardware_interface::HW_IF_POSITION;
using hardware_interface::HW_IF_VELOCITY;

namespace
{
class TaskDiffDrive : public diff_drive_controller::DiffDriveController
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { received_velocity_msg_.set(m); }
  double x() const { return odometry_.getX(); }
  double heading() const { return odometry_.getHeading(); }
};

// From the unpatched upstream code: left and right wheel command, odometry x
// and heading after the guarded cycles.
constexpr double kExpected[4] = {
  1.5200000000000029, 6.5200000000000031, 0.014999750001249999, 0.010000000000000002};
}  // namespace

class DiffDriveHistoryTask : public ::testing::Test
{
protected:
  void SetUp() override
  {
    controller_ = std::make_unique<TaskDiffDrive>();
    left_pos_ = std::make_shared<hardware_interface::StateInterface>("left_wheel_joint", HW_IF_POSITION);
    right_pos_ = std::make_shared<hardware_interface::StateInterface>("right_wheel_joint", HW_IF_POSITION);
    left_cmd_ = std::make_shared<hardware_interface::CommandInterface>("left_wheel_joint", HW_IF_VELOCITY);
    right_cmd_ = std::make_shared<hardware_interface::CommandInterface>("right_wheel_joint", HW_IF_VELOCITY);
    std::ignore = left_pos_->set_value(0.1);
    std::ignore = right_pos_->set_value(0.2);
    std::ignore = left_cmd_->set_value(0.0);
    std::ignore = right_cmd_->set_value(0.0);

    rclcpp::NodeOptions options;
    options.parameter_overrides({
      rclcpp::Parameter("left_wheel_names", std::vector<std::string>{"left_wheel_joint"}),
      rclcpp::Parameter("right_wheel_names", std::vector<std::string>{"right_wheel_joint"}),
      rclcpp::Parameter("wheel_separation", 1.0),
      rclcpp::Parameter("wheel_radius", 0.1),
      rclcpp::Parameter("linear.x.has_acceleration_limits", true),
      rclcpp::Parameter("linear.x.has_jerk_limits", true),
      rclcpp::Parameter("linear.x.max_acceleration", 2.0),
      rclcpp::Parameter("linear.x.max_jerk", 400.0),
      rclcpp::Parameter("angular.z.has_acceleration_limits", true),
      rclcpp::Parameter("angular.z.has_jerk_limits", true),
      rclcpp::Parameter("angular.z.max_acceleration", 3.0),
      rclcpp::Parameter("angular.z.max_jerk", 400.0),
    });
    controller_interface::ControllerInterfaceParams params;
    params.controller_name = "diff_drive_controller";
    params.robot_description = "";
    params.update_rate = 0;
    params.node_namespace = "";
    params.node_options = options;
    ASSERT_EQ(controller_->init(params), controller_interface::return_type::OK);

    std::vector<hardware_interface::LoanedStateInterface> state_ifs;
    state_ifs.emplace_back(left_pos_);
    state_ifs.emplace_back(right_pos_);
    std::vector<hardware_interface::LoanedCommandInterface> cmd_ifs;
    cmd_ifs.emplace_back(left_cmd_, nullptr);
    cmd_ifs.emplace_back(right_cmd_, nullptr);
    controller_->assign_interfaces(std::move(cmd_ifs), std::move(state_ifs));
    controller_->export_reference_interfaces();
  }

  void TearDown() override { controller_.reset(); }

  std::unique_ptr<TaskDiffDrive> controller_;
  hardware_interface::StateInterface::SharedPtr left_pos_, right_pos_;
  hardware_interface::CommandInterface::SharedPtr left_cmd_, right_cmd_;
};

TEST_F(DiffDriveHistoryTask, UpdateDoesNotAllocate)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  const auto period = rclcpp::Duration::from_seconds(0.001);
  rclcpp::Time t(0, 0, RCL_ROS_TIME);
  for (int k = 0; k < 3; ++k)
  {
    controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.0, 0.5, t));
    ASSERT_EQ(controller_->update(t, period), controller_interface::return_type::OK);
    t = t + period;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  const auto r = rtswe_eval::run_probe(
    "diff_drive_controller", "rt-007: limits on, short frame, streamed twist, warm-up 3", 200,
    [&](const rclcpp::Time & tt, const rclcpp::Duration & p) {return controller_->update(tt, p);},
    0.001, t,
    [&](int, const rclcpp::Time & tt) {
      controller_->push(rtswe_eval::twist(rtswe_eval::kShortFrame, 1.0, 0.0, 0.5, tt));
    });
  rtswe_eval::print(r);

  const double got[4] = {
    left_cmd_->get_optional().value_or(std::numeric_limits<double>::quiet_NaN()),
    right_cmd_->get_optional().value_or(std::numeric_limits<double>::quiet_NaN()),
    controller_->x(), controller_->heading()};
  bool match = true;
  std::cout << "[rtswe] values=" << std::setprecision(17);
  for (std::size_t i = 0; i < 4; ++i)
  {
    std::cout << got[i] << (i < 3 ? "," : "\n");
    match = match && std::abs(got[i] - kExpected[i]) < 1e-9;
  }
  rtswe_eval::report_functional(r.failed_cycles == 0 && match);
  EXPECT_TRUE(match);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
