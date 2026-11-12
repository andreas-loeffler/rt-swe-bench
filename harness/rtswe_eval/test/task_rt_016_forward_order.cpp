// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-016 (forward_command_controller, Track 1, hand-written
// bug). The controller must write command i to the interface of joint i, in
// the order of the `joints` parameter, whatever order the interfaces were
// assigned in.
//
// 12 joints with names longer than the 15-char small-string buffer, position
// interfaces, assigned in reverse order. No warm-up (the controller has no
// publisher). 60 guarded cycles; commands arrive outside the guard, as from the
// subscriber thread: 10 + i for joint i from cycle 0, 20 + i from cycle 30.
// The commanded values are checked before cycle 30 and after the last cycle.

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

forward_command_controller::CmdType command(double base)
{
  forward_command_controller::CmdType c;
  for (std::size_t i = 0; i < kJoints; ++i) {c.data.push_back(base + static_cast<double>(i));}
  return c;
}
}  // namespace

class ForwardOrderTask : public ::testing::Test
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
    // assigned in reverse order of the `joints` parameter
    for (auto it = cmd_ifs_.rbegin(); it != cmd_ifs_.rend(); ++it) {loaned.emplace_back(*it, nullptr);}
    controller_->assign_interfaces(std::move(loaned), {});
    controller_->get_node()->set_parameter({"joints", joint_names_});
    controller_->get_node()->set_parameter({"interface_name", "position"});
  }

  bool values_are(double base) const
  {
    bool ok = true;
    for (std::size_t i = 0; i < kJoints; ++i)
    {
      ok = ok && cmd_ifs_[i]->get_optional().value_or(kNaN) == base + static_cast<double>(i);
    }
    return ok;
  }

  std::vector<std::string> joint_names_;
  std::vector<hardware_interface::CommandInterface::SharedPtr> cmd_ifs_;
  std::unique_ptr<TaskController> controller_;
};

TEST_F(ForwardOrderTask, UpdateDoesNotAllocate)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  bool before_30 = false;
  controller_->push(command(10.0));
  const auto r = rtswe_eval::run_probe(
    "forward_command_controller", "rt-016: 12 joints, interfaces in reverse order", 60,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.001, rclcpp::Time(0, 0, RCL_ROS_TIME),
    [&](int k, const rclcpp::Time &) {
      if (k == 30)
      {
        before_30 = values_are(10.0);
        controller_->push(command(20.0));
      }
    });
  rtswe_eval::print(r);
  const bool after = values_are(20.0);
  std::cout << "[rtswe] before_30=" << before_30 << " after=" << after << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && before_30 && after);
  EXPECT_TRUE(before_30);
  EXPECT_TRUE(after);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}
