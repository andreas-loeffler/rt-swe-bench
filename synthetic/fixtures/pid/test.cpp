// Tier 1 test: pid. 1,000 control steps for 6 DOFs (integral saturates), checked
// against an independent computation.
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "kernel.hpp"
#include "rtk/check.hpp"

int main()
{
  rtk::setup();
  const std::size_t dofs = 6;
  const double kp = 2.0, ki = 0.8, kd = 0.05, dt = 0.001, ilim = 0.3;
  Pid pid;
  pid.configure(dofs, kp, ki, kd, dt, ilim);
  std::vector<double> ref(dofs), meas(dofs), cmd(dofs), integ(dofs, 0.0), prev(dofs, 0.0);
  rtk::Run run;
  for (int k = 0; k < 1000; ++k)
  {
    for (std::size_t i = 0; i < dofs; ++i)
    {
      ref[i] = 1.0 + 0.2 * static_cast<double>(i);
      meas[i] = std::sin(0.002 * k * static_cast<double>(i + 1));
    }
    rtk::guarded_void(run, [&] { pid.process(ref.data(), meas.data(), cmd.data()); });
    for (std::size_t i = 0; i < dofs; ++i)
    {
      const double e = ref[i] - meas[i];
      integ[i] = std::clamp(integ[i] + ki * e * dt, -ilim, ilim);
      const double d = k == 0 ? 0.0 : (e - prev[i]) / dt;
      const double expected = kp * e + integ[i] + kd * d;
      prev[i] = e;
      rtk::expect(run, rtk::near(cmd[i], expected, 1e-9), "step " + std::to_string(k) + " dof " + std::to_string(i));
    }
  }
  return rtk::report(run);
}
