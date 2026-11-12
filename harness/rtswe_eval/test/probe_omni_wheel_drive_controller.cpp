// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: unmodified OmniWheelDriveController with the upstream test
// fixture (4 wheels, position feedback). A fresh TwistStamped is stored before
// every cycle, outside the guard, as a subscriber streaming commands would.

#include <string>

#include "rtswe_eval/probe_main.hpp"
#include "test_omni_wheel_drive_controller.hpp"  // upstream fixture

constexpr int kCensusCycles = 200;  // long enough to show periodic allocations

namespace
{
class ProbeOmni : public TestableOmniWheelDriveController
{
public:
  void push(const geometry_msgs::msg::TwistStamped & m) { received_velocity_msg_.set(m); }
};
}  // namespace

class OmniProbe : public OmniWheelDriveControllerFixture<ProbeOmni>
{
protected:
  void run(const std::string & frame_id)
  {
    ASSERT_EQ(InitController(), controller_interface::return_type::OK);
    ASSERT_TRUE(configure_succeeds(controller_));
    controller_->export_reference_interfaces();
    assignResourcesPosFeedback();
    ASSERT_TRUE(activate_succeeds(controller_));
    controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.5, 0.2));
    const auto r = rtswe_eval::run_probe(
      "omni_wheel_drive_controller", rtswe_eval::frame_config(frame_id), kCensusCycles,
      [&](const rclcpp::Time & t, const rclcpp::Duration & p) {return controller_->update(t, p);},
      0.001, rclcpp::Time(0, 0, RCL_ROS_TIME),
      // a fresh command every cycle, stored outside the guard like a subscriber would
      [&](int, const rclcpp::Time & t) {controller_->push(rtswe_eval::twist(frame_id, 1.0, 0.5, 0.2, t));});
    rtswe_eval::print(r);
    EXPECT_EQ(r.failed_cycles, 0);
    EXPECT_EQ(r.allocating_cycles, 0);
  }
};

TEST_F(OmniProbe, CensusShortFrame) { run(rtswe_eval::kShortFrame); }
TEST_F(OmniProbe, CensusLongFrame) { run(rtswe_eval::kLongFrame); }

RTSWE_PROBE_MAIN()
