// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-002 (pid_controller, Track 2). Upstream fixture and
// parameters: one DoF, p=1 i=2 d=3, output clamped to [-5, 5].
//
// Warm-up: three unguarded update() calls after activation. Their one-time allocations
// (parameter refresh, first state publish) are outside this task. Then a
// reference is stored, as the subscriber would, and 50 cycles are guarded.

#include <cmath>
#include <limits>

#include "rtswe_eval/probe_main.hpp"
#include "test_pid_controller.hpp"  // upstream fixture, pid_controller/test/

class PidTask : public PidControllerFixture<TestablePidController>
{
};

TEST_F(PidTask, UpdateDoesNotAllocate)
{
  SetUpController("test_pid_controller");
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->set_chained_mode(false);
  ASSERT_TRUE(activate_succeeds(controller_));

  const auto period = rclcpp::Duration::from_seconds(0.01);
  const rclcpp::Time t0(0, 0, RCL_ROS_TIME);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    t0, period);
  ASSERT_TRUE(start.has_value());

  controller_->set_reference(dof_command_values_);  // 101.101, outside the guard
  const auto r = rtswe_eval::run_probe(
    "pid_controller", "rt-002: 1 dof, subscriber mode, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01, *start);
  rtswe_eval::print(r);

  // Reference 101.101 against state 1.1 saturates the output at u_clamp_max.
  const double out =
    command_itfs_[0]->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
  const bool saturated = std::abs(out - 5.0) < 1e-9;
  std::cout << "[rtswe] command=" << out << "\n";
  rtswe_eval::report_functional(r.failed_cycles == 0 && saturated);
  EXPECT_TRUE(saturated);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
