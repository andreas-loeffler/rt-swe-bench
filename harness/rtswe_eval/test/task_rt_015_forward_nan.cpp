// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-015 (forward_command_controller, Track 1, hand-written
// bug). A NaN entry in a command means "no new command for this joint": the
// joint keeps its current command instead of receiving NaN.
//
// 12 joints with names longer than the 15-char small-string buffer, position
// interfaces. No warm-up (the controller has no publisher). 60 guarded cycles;
// commands arrive outside the guard, as from the subscriber thread:
//   cycle  0: all joints 0.5
//   cycle 20: even joints 1.0, odd joints NaN   -> odd joints stay at 0.5
//   cycle 40: odd joints 2.0, even joints NaN   -> even joints stay at 1.0
// The commanded values are checked before cycles 20 and 40 and after the last.

#include <gmock/gmock.h>

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "forward_command_controller/forward_command_controller.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rtswe_eval/probe.hpp"

namespace
{
constexpr std::size_t kJoints = 12;
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

class TaskController : public forward_command_controller::ForwardCommandController
{
public:
  void push(const forward_command_controller::CmdType & msg) { rt_command_.set(msg); }
};

forward_command_controller::CmdType command(double even, double odd)
{
  forward_command_controller::CmdType c;
  for (std::size_t i = 0; i < kJoints; ++i) {c.data.push_back(i % 2 == 0 ? even : odd);}
  return c;
}
}  // namespace

class ForwardNanTask : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    rclcpp::init(0, nullptr);
    rtswe_eval::install_hooks();
  }
  static void TearDownTestSuite() { rclcpp::shutdown(); }

  void SetUp() override
  {
    for (std::size_t i = 0; i < kJoints; ++i)
    {
      joint_names_.push_back("long_robot_arm_joint_" + std::to_string(i));
      cmd_ifs_.push_back(std::make_shared<hardware_interface::CommandInterface>(
        joint_names_.back(), hardware_interface::HW_IF_POSITION));
      std::ignore = cmd_ifs_.back()->set_value(0.0);
    }
    controller_ = std::make_unique<TaskController>();
    controller_interface::ControllerInterfaceParams params;
    params.controller_name = "forward_command_controller";
    params.robot_description = "";
    params.update_rate = 0;
    params.node_namespace = "";
    params.node_options = controller_->define_custom_node_options();
    ASSERT_EQ(controller_->init(params), controller_interface::return_type::OK);

    std::vector<hardware_interface::LoanedCommandInterface> loaned;
    for (auto & ci : cmd_ifs_) {loaned.emplace_back(ci, nullptr);}
    controller_->assign_interfaces(std::move(loaned), {});
    controller_->get_node()->set_parameter({"joints", joint_names_});
    controller_->get_node()->set_parameter({"interface_name", "position"});
  }

  bool values_are(double even, double odd) const
  {
    bool ok = true;
    for (std::size_t i = 0; i < kJoints; ++i)
    {
      const double v = cmd_ifs_[i]->get_optional().value_or(kNaN);
      ok = ok && v == (i % 2 == 0 ? even : odd);
    }
    return ok;
  }

  std::vector<std::string> joint_names_;
  std::vector<hardware_interface::CommandInterface::SharedPtr> cmd_ifs_;
  std::unique_ptr<TaskController> controller_;
};

TEST_F(ForwardNanTask, UpdateDoesNotAllocate)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  bool before_20 = false, before_40 = false;
  controller_->push(command(0.5, 0.5));
  const auto r = rtswe_eval::run_probe(
    "forward_command_controller", "rt-015: 12 joints, partial NaN commands", 60,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.001, rclcpp::Time(0, 0, RCL_ROS_TIME),
    [&](int k, const rclcpp::Time &) {
      if (k == 20)
      {
        before_20 = values_are(0.5, 0.5);
        controller_->push(command(1.0, kNaN));
      }
      else if (k == 40)
      {
        before_40 = values_are(1.0, 0.5);
        controller_->push(command(kNaN, 2.0));
      }
    });
  rtswe_eval::print(r);
  const bool after = values_are(1.0, 2.0);
  std::cout << "[rtswe] before_20=" << before_20 << " before_40=" << before_40 << " after=" << after
            << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && before_20 && before_40 && after);
  EXPECT_TRUE(before_20);
  EXPECT_TRUE(before_40);
  EXPECT_TRUE(after);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}
