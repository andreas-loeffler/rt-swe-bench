// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified MecanumDriveController with the upstream test
// fixture and parameter file. One TwistStamped is stored before the loop, as
// the subscriber would.

#include <string>

#include "rtswe_eval/probe_main.hpp"
#include "test_mecanum_drive_controller.hpp"  // upstream fixture

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

namespace
{
class ProbeMecanum : public TestableMecanumDriveController
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { input_ref_.set(m); }
};
}  // namespace

class MecanumProbe : public MecanumDriveControllerFixture<ProbeMecanum>
{
protected:
  void run(const std::string & frame_id)
  {
    SetUpController();
    ASSERT_TRUE(configure_succeeds(controller_));
    ASSERT_TRUE(activate_succeeds(controller_));
    controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.5, 0.2));
    const auto r = rtswe_eval::run_probe(
      "mecanum_drive_controller", rtswe_eval::frame_config(frame_id), kCensusCycles,
      [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
      0.001, rclcpp::Time(0, 0, RCL_ROS_TIME),
      // a fresh command every cycle, stored outside the guard like a subscriber would
      [&](int, const rclcpp::Time & t) {controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.5, 0.2, t));});
    rtswe_eval::print(r);
    EXPECT_EQ(r.failed_cycles, 0);
    EXPECT_EQ(r.allocating_cycles, 0);
  }
};

TEST_F(MecanumProbe, CensusShortFrame) { run(rtswe_eval::kShortFrame); }
TEST_F(MecanumProbe, CensusLongFrame) { run(rtswe_eval::kLongFrame); }

RTSWE_PROBE_MAIN()
