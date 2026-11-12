// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified upstream PidController, set up with the upstream
// test fixture and parameter file (one DoF named "joint1", position
// interface). One reference is stored before the loop, as the subscriber
// would; then 200 guarded update() cycles.

#include <gmock/gmock.h>

#include "rtswe_eval/probe.hpp"
#include "test_pid_controller.hpp"  // upstream fixture, pid_controller/test/

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

class PidProbe : public PidControllerFixture<TestablePidController>
{
};

TEST_F(PidProbe, Census)
{
  rtswe_eval::install_hooks();
  SetUpController("test_pid_controller");
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->set_chained_mode(false);
  ASSERT_TRUE(activate_succeeds(controller_));
  controller_->set_reference(dof_command_values_);  // outside the guard

  const auto r = rtswe_eval::run_probe(
    "pid_controller", "upstream test params, 1 dof, subscriber mode", kCensusCycles,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01);
  rtswe_eval::print(r);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);
  int result = RUN_ALL_TESTS();
  rclcpp::shutdown();
  return result;
}
