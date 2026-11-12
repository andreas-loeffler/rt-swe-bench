// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified AdmittanceController with the upstream test fixture
// and parameters (6-DoF test robot, KDL kinematics). No command is sent; the
// controller holds the measured state. 200 guarded update() cycles.

#include "rtswe_eval/probe_main.hpp"
#include "test_admittance_controller.hpp"  // upstream fixture

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

class AdmittanceProbe : public AdmittanceControllerTest
{
};

TEST_F(AdmittanceProbe, Census)
{
  ASSERT_EQ(SetUpController(), controller_interface::return_type::OK);
  ASSERT_TRUE(configure_succeeds(controller_));
  ASSERT_TRUE(activate_succeeds(controller_));
  broadcast_tfs();
  const auto r = rtswe_eval::run_probe(
    "admittance_controller", "upstream test params, 6-DoF robot, no command", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01);
  rtswe_eval::print(r);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
