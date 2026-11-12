// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-011 (pid_controller, Track 2). When a parameter changes
// at runtime, update() refreshes its parameters with a blocking is_old() and
// a by-value get_params(), which copies the whole Params struct (strings,
// maps) on the real-time thread.
//
// Upstream fixture and parameters, one DoF, chained mode: the reference comes
// through the reference interface, so the subscriber path (rt-002) is not
// involved. Warm-up: three unguarded cycles. Then 50 guarded cycles; before
// cycle 10 the proportional gain is changed, outside the guard, as a parameter
// service call would. The output must follow the new gain, without allocating.

#include <cmath>
#include <iomanip>
#include <limits>

#include "rtswe_eval/probe_main.hpp"
#include "test_pid_controller.hpp"  // upstream fixture, pid_controller/test/

namespace
{
class TaskPid : public TestablePidController
{
public:
  void set_reference_interface(double value)
  {
    ordered_exported_reference_interfaces_[0]->set_value(value);
  }
};

// From the unpatched upstream code: the command before the gain change (after
// guarded cycle 9) and after the last guarded cycle.
constexpr double kExpected[2] = {1.1159999999999999, 2.7360000000000002};
}  // namespace

class PidParamsTask : public PidControllerFixture<TaskPid>
{
};

TEST_F(PidParamsTask, ParameterChangeDoesNotAllocate)
{
  SetUpController("test_pid_controller_unlimited");
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->set_chained_mode(true);
  ASSERT_TRUE(activate_succeeds(controller_));
  ASSERT_TRUE(controller_->is_in_chained_mode());
  auto * pid = static_cast<TaskPid *>(controller_.get());  // the fixture creates a TaskPid

  const auto period = rclcpp::Duration::from_seconds(0.01);
  pid->set_reference_interface(2.0);
  const auto start = rtswe_eval::warm_up(
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    rclcpp::Time(0, 0, RCL_ROS_TIME), period);
  ASSERT_TRUE(start.has_value());

  double before_change = std::numeric_limits<double>::quiet_NaN();
  bool set_ok = true;
  const auto r = rtswe_eval::run_probe(
    "pid_controller", "rt-011: chained, p gain 1.0 -> 2.0 at cycle 10, warm-up 3", 50,
    [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
    0.01, *start,
    [&](int k, const rclcpp::Time &) {
      pid->set_reference_interface(2.0);
      if (k == 10)
      {
        before_change = command_itfs_[0]->get_optional().value_or(std::nan(""));
        set_ok = controller_->get_node()
                   ->set_parameter(rclcpp::Parameter("gains.joint1.p", 2.0))
                   .successful;
      }
    });
  rtswe_eval::print(r);

  const double after = command_itfs_[0]->get_optional().value_or(std::nan(""));
  std::cout << "[rtswe] values=" << std::setprecision(17) << before_change << "," << after << "\n";
  const bool match = set_ok && std::abs(before_change - kExpected[0]) < 1e-9 &&
                     std::abs(after - kExpected[1]) < 1e-9;
  rtswe_eval::report_functional(r.failed_cycles == 0 && match);
  EXPECT_TRUE(set_ok);
  EXPECT_TRUE(match);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
