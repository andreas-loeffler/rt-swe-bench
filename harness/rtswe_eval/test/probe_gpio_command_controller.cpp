// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified GpioCommandController with two GPIO groups and
// realistic, longer interface names (tool flange digital outputs and an
// analog output), command and state interfaces for each. One command is
// stored before the loop, as the subscriber would. 200 guarded cycles.

#include <memory>
#include <string>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "gpio_controllers/gpio_command_controller.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "rtswe_eval/probe_main.hpp"

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

namespace
{
class ProbeGpio : public gpio_controllers::GpioCommandController
{
public:
  void push(const gpio_controllers::CmdType & m) { rt_command_.set(m); }
};

const std::vector<std::string> kGroups = {"tool_flange_io", "cabinet_analog_io"};
const std::vector<std::vector<std::string>> kInterfaces = {
  {"digital_output_1", "digital_output_2"}, {"analog_output_1"}};
}  // namespace

class GpioProbe : public ::testing::Test
{
protected:
  void SetUp() override
  {
    controller_ = std::make_unique<ProbeGpio>();
    for (std::size_t g = 0; g < kGroups.size(); ++g)
    {
      for (const auto & itf : kInterfaces[g])
      {
        cmds_.push_back(std::make_shared<hardware_interface::CommandInterface>(kGroups[g], itf));
        std::ignore = cmds_.back()->set_value(0.0);
        states_.push_back(std::make_shared<hardware_interface::StateInterface>(kGroups[g], itf));
        std::ignore = states_.back()->set_value(0.0);
      }
    }
    rclcpp::NodeOptions options;
    options.parameter_overrides({
      rclcpp::Parameter("gpios", kGroups),
      rclcpp::Parameter("command_interfaces." + kGroups[0] + ".interfaces", kInterfaces[0]),
      rclcpp::Parameter("command_interfaces." + kGroups[1] + ".interfaces", kInterfaces[1]),
      rclcpp::Parameter("state_interfaces." + kGroups[0] + ".interfaces", kInterfaces[0]),
      rclcpp::Parameter("state_interfaces." + kGroups[1] + ".interfaces", kInterfaces[1]),
    });
    controller_interface::ControllerInterfaceParams params;
    params.controller_name = "gpio_command_controller";
    params.robot_description = "";
    params.update_rate = 0;
    params.node_namespace = "";
    params.node_options = options;
    ASSERT_EQ(controller_->init(params), controller_interface::return_type::OK);
  }

  void assign()
  {
    std::vector<hardware_interface::LoanedCommandInterface> c;
    for (auto & x : cmds_) {c.emplace_back(x, nullptr);}
    std::vector<hardware_interface::LoanedStateInterface> s;
    for (auto & x : states_) {s.emplace_back(x);}
    controller_->assign_interfaces(std::move(c), std::move(s));
  }

  std::unique_ptr<ProbeGpio> controller_;
  std::vector<hardware_interface::CommandInterface::SharedPtr> cmds_;
  std::vector<hardware_interface::StateInterface::SharedPtr> states_;
};

TEST_F(GpioProbe, Census)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  assign();
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  gpio_controllers::CmdType cmd;
  cmd.interface_groups = kGroups;
  cmd.interface_values.resize(2);
  cmd.interface_values[0].interface_names = kInterfaces[0];
  cmd.interface_values[0].values = {1.0, 0.0};
  cmd.interface_values[1].interface_names = kInterfaces[1];
  cmd.interface_values[1].values = {4.2};
  controller_->push(cmd);

  const auto r = rtswe_eval::run_probe(
    "gpio_command_controller", "2 groups, 3 interfaces, long names, one command", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01);
  rtswe_eval::print(r);
  EXPECT_EQ(cmds_[2]->get_optional().value_or(0.0), 4.2);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
