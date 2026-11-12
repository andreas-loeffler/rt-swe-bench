// Tier 1 test: status_line. 1,000 real-time calls with joint names longer than
// the 15-character small-string buffer, compared with an independent formatting.
#include <charconv>
#include <cmath>
#include <string>
#include <vector>

#include "kernel.hpp"
#include "rtk/check.hpp"

static std::string fixed6(double v)
{
  char b[64];
  return std::string(b, std::to_chars(b, b + sizeof b, v, std::chars_format::fixed, 6).ptr);
}

int main()
{
  rtk::setup();
  StatusLine sl;
  sl.configure(256);
  const std::vector<std::string> joints = {"shoulder_pan_joint", "shoulder_lift_joint", "elbow_joint",
                                           "wrist_1_joint", "wrist_2_joint", "wrist_3_joint"};
  rtk::Run run;
  for (int i = 0; i < 1000; ++i)
  {
    const auto & joint = joints[i % joints.size()];
    const double pos = std::sin(0.01 * i) * 3.0, vel = std::cos(0.01 * i) * -1.5;
    const auto id = static_cast<std::uint32_t>(100000 + i);
    const std::string_view got = rtk::guarded(run, [&] { return sl.format(id, joint, pos, vel); });
    const std::string expected = "id=" + std::to_string(id) + " joint=" + joint + " pos=" + fixed6(pos) +
                                 " vel=" + fixed6(vel);
    rtk::expect(run, got == expected, "call " + std::to_string(i) + ": '" + std::string(got) + "'");
  }
  return rtk::report(run);
}
