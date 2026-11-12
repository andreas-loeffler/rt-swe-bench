// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Shared pieces for census probes: a gtest main that initializes rclcpp with
// the command-line arguments (so --params-file reaches the controller node)
// and installs the malloc-guard hooks, plus message helpers.

#ifndef RTSWE_EVAL__PROBE_MAIN_HPP_
#define RTSWE_EVAL__PROBE_MAIN_HPP_

#include <gmock/gmock.h>

#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rtswe_eval/probe.hpp"

namespace rtswe_eval
{

// Frame ids for the two probe configurations. "base_link" fits the 15-char
// small-string buffer; a namespaced multi-robot frame does not.
inline const std::string kShortFrame = "base_link";
inline const std::string kLongFrame = "robot_1/base_link";

inline geometry_msgs::msg::TwistStamped twist(
  const std::string & frame_id, double vx, double vy, double wz,
  const rclcpp::Time & stamp = rclcpp::Time(0, 0, RCL_ROS_TIME))
{
  geometry_msgs::msg::TwistStamped m;
  m.header.stamp = stamp;
  m.header.frame_id = frame_id;
  m.twist.linear.x = vx;
  m.twist.linear.y = vy;
  m.twist.angular.z = wz;
  return m;
}

inline std::string frame_config(const std::string & frame_id)
{
  return "frame_id \"" + frame_id + "\" (" + std::to_string(frame_id.size()) + " chars)";
}

}  // namespace rtswe_eval

#define RTSWE_PROBE_MAIN()                                 \
  int main(int argc, char ** argv)                         \
  {                                                        \
    ::testing::InitGoogleMock(&argc, argv);                \
    rclcpp::init(argc, argv);                              \
    rtswe_eval::install_hooks();                           \
    const int result = RUN_ALL_TESTS();                    \
    if (rclcpp::ok()) {rclcpp::shutdown();}                \
    return result;                                         \
  }

#endif  // RTSWE_EVAL__PROBE_MAIN_HPP_
