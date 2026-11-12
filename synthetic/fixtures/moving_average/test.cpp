// Tier 1 test: moving_average. 2,000 real-time calls with window 16, each checked
// against the mean computed independently from the input history.
#include <cmath>
#include <string>
#include <vector>

#include "kernel.hpp"
#include "rtk/check.hpp"

int main()
{
  rtk::setup();
  const std::size_t window = 16;
  MovingAverage ma;
  ma.configure(window);
  std::vector<double> history;
  history.reserve(2000);
  rtk::Run run;
  for (int i = 0; i < 2000; ++i)
  {
    const double x = std::sin(0.1 * i) + 0.01 * i;
    history.push_back(x);
    const double y = rtk::guarded(run, [&] { return ma.process(x); });
    const std::size_t n = std::min(history.size(), window);
    double expected = 0.0;
    for (std::size_t k = history.size() - n; k < history.size(); ++k) {expected += history[k];}
    expected /= static_cast<double>(n);
    rtk::expect(run, rtk::near(y, expected, 1e-9), "call " + std::to_string(i));
  }
  return rtk::report(run);
}
