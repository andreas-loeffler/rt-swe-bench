// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Probe: drive the *unmodified* upstream ForwardCommandController through its
// real lifecycle (configure, activate) and run 200 update() cycles with
// malloc-guard armed around each call. Reports every allocation by site.

#include <gmock/gmock.h>

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

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

namespace
{
// Exposes the command box so the test can inject a command without a topic.
class ProbeController : public forward_command_controller::ForwardCommandController
{
public:
  void push(const forward_command_controller::CmdType & msg) { rt_command_.set(msg); }
};
}  // namespace

class ProbeForwardCommandController : public ::testing::Test
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
    // 12 joints with names longer than the 15-char small-string buffer.
    for (int i = 0; i < 12; ++i)
    {
      joint_names_.push_back("long_robot_arm_joint_" + std::to_string(i));
      cmd_ifs_.push_back(std::make_shared<hardware_interface::CommandInterface>(
        joint_names_.back(), hardware_interface::HW_IF_POSITION));
      std::ignore = cmd_ifs_.back()->set_value(0.0);
    }
    controller_ = std::make_unique<ProbeController>();
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

  std::vector<std::string> joint_names_;
  std::vector<hardware_interface::CommandInterface::SharedPtr> cmd_ifs_;
  std::unique_ptr<ProbeController> controller_;
};

TEST_F(ProbeForwardCommandController, Census)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  forward_command_controller::CmdType cmd;
  cmd.data.assign(joint_names_.size(), 0.5);
  controller_->push(cmd);  // outside the guard, like the subscriber thread

  const auto r = rtswe_eval::run_probe(
    "forward_command_controller", "12 joints, position", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);});
  rtswe_eval::print(r);
  // functional check: every cycle returned OK and the command reached every interface
  bool reached = true;
  for (const auto & ci : cmd_ifs_) {reached = reached && ci->get_optional().value_or(0.0) == 0.5;}
  rtswe_eval::report_functional(r.failed_cycles == 0 && reached);
  EXPECT_TRUE(reached);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}
