// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified JointStateBroadcaster, set up like the upstream
// test (three joints with position, velocity and effort state interfaces,
// default parameters). It publishes joint_states and dynamic_joint_states
// from update(). 200 guarded update() cycles.

#include <memory>
#include <string>
#include <vector>

#include "controller_interface/test_utils.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "joint_state_broadcaster/joint_state_broadcaster.hpp"
#include "rtswe_eval/probe_main.hpp"

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

class JsbProbe : public ::testing::Test
{
protected:
  void SetUp() override
  {
    broadcaster_ = std::make_unique<joint_state_broadcaster::JointStateBroadcaster>();
    const std::vector<std::string> joints = {"joint1", "joint2", "joint3"};
    const std::vector<std::string> types = {
      hardware_interface::HW_IF_POSITION, hardware_interface::HW_IF_VELOCITY,
      hardware_interface::HW_IF_EFFORT};
    double v = 1.1;
    for (const auto & type : types)
    {
      for (const auto & joint : joints)
      {
        states_.push_back(std::make_shared<hardware_interface::StateInterface>(joint, type));
        std::ignore = states_.back()->set_value(v);
        v += 1.0;
      }
    }
    controller_interface::ControllerInterfaceParams params;
    params.controller_name = "joint_state_broadcaster";
    params.robot_description = "";
    params.update_rate = 0;
    params.node_namespace = "";
    params.node_options = broadcaster_->define_custom_node_options();
    ASSERT_EQ(broadcaster_->init(params), controller_interface::return_type::OK);
    std::vector<hardware_interface::LoanedStateInterface> loaned;
    for (auto & s : states_) {loaned.emplace_back(s);}
    broadcaster_->assign_interfaces({}, std::move(loaned));
  }

  std::unique_ptr<joint_state_broadcaster::JointStateBroadcaster> broadcaster_;
  std::vector<hardware_interface::StateInterface::SharedPtr> states_;
};

TEST_F(JsbProbe, Census)
{
  ASSERT_TRUE(controller_interface::configure_succeeds(broadcaster_));
  ASSERT_TRUE(controller_interface::activate_succeeds(broadcaster_));
  const auto r = rtswe_eval::run_probe(
    "joint_state_broadcaster", "3 joints x position/velocity/effort, defaults", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return broadcaster_->update(t, p);},
    0.01);
  rtswe_eval::print(r);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
