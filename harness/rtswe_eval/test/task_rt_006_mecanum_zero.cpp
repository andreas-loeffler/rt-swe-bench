// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Hidden test for rt-006 (mecanum_drive_controller, Track 1). Task base: the
// fix of ros2_controllers#2624 reverted, so a NaN reference zeroes only the
// front-left wheel (the four set_value() calls were chained with ||).
//
// Chained mode, upstream fixture and parameters. Warm-up: three unguarded
// cycles in which the preceding controller writes a non-zero reference, so all
// wheels turn. Then 50 guarded cycles without a new reference: the controller
// resets its references to NaN after every tick, so each guarded cycle takes
// the zeroing branch. Every wheel must be commanded to zero, without
// allocating.

#include <chrono>
#include <cmath>
#include <limits>
#include <thread>

#include "rtswe_eval/probe_main.hpp"
#include "test_mecanum_drive_controller.hpp"  // upstream fixture

namespace
{
class TaskMecanum : public TestableMecanumDriveController
{
public:
  void set_reference(double vx, double vy, double wz)
  {
    ordered_exported_reference_interfaces_[0]->set_value(vx);
    ordered_exported_reference_interfaces_[1]->set_value(vy);
    ordered_exported_reference_interfaces_[2]->set_value(wz);
  }
};
}  // namespace

class MecanumZeroTask : public MecanumDriveControllerFixture<TaskMecanum>
{
};

TEST_F(MecanumZeroTask, NanReferenceZeroesAllWheelsWithoutAllocating)
{
  SetUpController();
  ASSERT_TRUE(configure_succeeds(controller_));
  controller_->set_chained_mode(true);
  ASSERT_TRUE(activate_succeeds(controller_));
  ASSERT_TRUE(controller_->is_in_chained_mode());

  const auto period = rclcpp::Duration::from_seconds(0.01);
  rclcpp::Time t(0, 0, RCL_ROS_TIME);
  bool moving = true;
  for (int k = 0; k < 3; ++k)
  {
    controller_->set_reference(1.0, 0.5, 0.25);
    ASSERT_EQ(controller_->update(t, period), controller_interface::return_type::OK);
    t = t + period;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  for (const auto & itf : command_itfs_)
  {
    moving = moving && itf->get_optional().value_or(0.0) != 0.0;
  }
  ASSERT_TRUE(moving) << "warm-up should leave every wheel turning";

  const auto r = rtswe_eval::run_probe(
    "mecanum_drive_controller", "rt-006: chained, NaN reference after motion, warm-up 3", 50,
    [&](const rclcpp::Time & tt, const rclcpp::Duration & p) {return controller_->update(tt, p);},
    0.01, t);
  rtswe_eval::print(r);

  bool all_zero = command_itfs_.size() == 4;
  std::cout << "[rtswe] wheels=";
  for (std::size_t i = 0; i < command_itfs_.size(); ++i)
  {
    const double v =
      command_itfs_[i]->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
    std::cout << v << (i + 1 < command_itfs_.size() ? "," : "\n");
    all_zero = all_zero && v == 0.0;
  }
  rtswe_eval::report_functional(r.failed_cycles == 0 && all_zero);
  EXPECT_TRUE(all_zero);
  EXPECT_EQ(r.failed_cycles, 0);
  EXPECT_EQ(r.allocating_cycles, 0);
}

RTSWE_PROBE_MAIN()
