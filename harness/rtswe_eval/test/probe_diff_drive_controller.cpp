// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified DiffDriveController, set up like the upstream test
// (one wheel per side, position feedback, open default parameters). One
// TwistStamped is stored before the loop, as the subscriber would.

#include <memory>
#include <string>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "diff_drive_controller/diff_drive_controller.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rtswe_eval/probe_main.hpp"

using hardware_interface::HW_IF_POSITION;
using hardware_interface::HW_IF_VELOCITY;

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

namespace
{
class ProbeDiffDrive : public diff_drive_controller::DiffDriveController
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { received_velocity_msg_.set(m); }
};
}  // namespace

class DiffDriveProbe : public ::testing::Test
{
protected:
  void SetUp() override
  {
    controller_ = std::make_unique<ProbeDiffDrive>();
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

  void run(const std::string & frame_id)
  {
    ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
    ASSERT_TRUE(controller_interface::activate_succeeds(controller_));
    controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.0, 0.5));
    const auto r = rtswe_eval::run_probe(
      "diff_drive_controller", rtswe_eval::frame_config(frame_id), kCensusCycles,
      [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
      0.001, rclcpp::Time(0, 0, RCL_ROS_TIME),
      // a fresh command every cycle, stored outside the guard like a subscriber would
      [&](int, const rclcpp::Time & t) {controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.0, 0.5, t));});
    rtswe_eval::print(r);
    EXPECT_NE(left_cmd_->get_optional().value(), 0.0);  // the twist reached the wheels
    EXPECT_EQ(r.failed_cycles, 0);
    EXPECT_EQ(r.allocating_cycles, 0);
  }

  std::unique_ptr<ProbeDiffDrive> controller_;
  hardware_interface::StateInterface::SharedPtr left_pos_, right_pos_;
  hardware_interface::CommandInterface::SharedPtr left_cmd_, right_cmd_;
};

TEST_F(DiffDriveProbe, CensusShortFrame) { run(rtswe_eval::kShortFrame); }
TEST_F(DiffDriveProbe, CensusLongFrame) { run(rtswe_eval::kLongFrame); }

RTSWE_PROBE_MAIN()
