// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-013 (gpio_controllers, Track 2). update() copies the
// whole command message out of the realtime box every cycle, and builds the
// full name "<group>/<interface>" as a new std::string for every command and
// state interface every cycle; with realistic names longer than the 15-char
// small-string buffer, each of those allocates.
//
// Two GPIO groups with longer names (tool flange digital outputs, a cabinet
// analog output), command and state interfaces for each. A command is stored
// before the warm-up, as the subscriber would. Warm-up: three unguarded
// cycles. Then 50 guarded cycles. Every command interface must hold its
// commanded value and the published state message the measured values.

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "gpio_controllers/gpio_command_controller.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "rtswe_eval/probe_main.hpp"

namespace
{
class TaskGpio : public gpio_controllers::GpioCommandController
{
public:
  void push(const gpio_controllers::CmdType & m) { rt_command_.set(m); }
  const control_msgs::msg::DynamicInterfaceGroupValues & state_msg() const
  {
    return gpio_state_msg_;
  }
};

const std::vector<std::string> kGroups = {"tool_flange_io", "cabinet_analog_io"};
const std::vector<std::vector<std::string>> kInterfaces = {
  {"digital_output_1", "digital_output_2"}, {"analog_output_1"}};
const std::vector<std::vector<double>> kCommands = {{1.0, 0.0}, {4.2}};
const std::vector<std::vector<double>> kStates = {{0.25, 0.75}, {3.5}};
}  // namespace

class GpioTask : public ::testing::Test
{
protected:
  void SetUp() override
  {
    controller_ = std::make_unique<TaskGpio>();
    for (std::size_t g = 0; g < kGroups.size(); ++g)
    {
      for (std::size_t i = 0; i < kInterfaces[g].size(); ++i)
      {
        cmds_.push_back(
          std::make_shared<hardware_interface::CommandInterface>(kGroups[g], kInterfaces[g][i]));
        std::ignore = cmds_.back()->set_value(-1.0);
        states_.push_back(
          std::make_shared<hardware_interface::StateInterface>(kGroups[g], kInterfaces[g][i]));
        std::ignore = states_.back()->set_value(kStates[g][i]);
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

  std::unique_ptr<TaskGpio> controller_;
  std::vector<hardware_interface::CommandInterface::SharedPtr> cmds_;
  std::vector<hardware_interface::StateInterface::SharedPtr> states_;
};

TEST_F(GpioTask, UpdateDoesNotAllocate)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(controller_));
  std::vector<hardware_interface::LoanedCommandInterface> c;
  for (auto & x : cmds_) {c.emplace_back(x, nullptr);}
  std::vector<hardware_interface::LoanedStateInterface> s;
  for (auto & x : states_) {s.emplace_back(x);}
  controller_->assign_interfaces(std::move(c), std::move(s));
  ASSERT_TRUE(controller_interface::activate_succeeds(controller_));

  gpio_controllers::CmdType cmd;
  cmd.interface_groups = kGroups;
  cmd.interface_values.resize(kGroups.size());
  for (std::size_t g = 0; g < kGroups.size(); ++g)
  {
    cmd.interface_values[g].interface_names = kInterfaces[g];
    cmd.interface_values[g].values = kCommands[g];
  }
  controller_->push(cmd);

  const auto period = rclcpp::Duration::from_seconds(0.01);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    rclcpp::Time(0, 0, RCL_ROS_TIME), period);
  ASSERT_TRUE(start.has_value());

  const auto r = rtswe_eval::run_probe(
    "gpio_command_controller", "rt-013: 2 groups, 3 interfaces, long names, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01, *start);
  rtswe_eval::print(r);

  bool commands_ok = true;
  bool states_ok = controller_->state_msg().interface_groups == kGroups &&
                   controller_->state_msg().interface_values.size() == kGroups.size();
  std::size_t k = 0;
  for (std::size_t g = 0; g < kGroups.size(); ++g)
  {
    for (std::size_t i = 0; i < kInterfaces[g].size(); ++i, ++k)
    {
      commands_ok = commands_ok && cmds_[k]->get_optional().value_or(std::nan("")) == kCommands[g][i];
      states_ok = states_ok &&
                  controller_->state_msg().interface_values[g].values.size() == kInterfaces[g].size() &&
                  controller_->state_msg().interface_values[g].values[i] == kStates[g][i];
    }
  }
  std::cout << "[rtswe] commands_ok=" << commands_ok << " states_ok=" << states_ok << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && commands_ok && states_ok);
  EXPECT_TRUE(commands_ok);
  EXPECT_TRUE(states_ok);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
