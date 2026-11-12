// Tier 1 test: median_filter. 2,000 real-time calls with window 9 on a noisy
// signal with spikes, against the lower median computed independently.
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "kernel.hpp"
#include "rtk/check.hpp"

int main()
{
  rtk::setup();
  const std::size_t window = 9;
  MedianFilter mf;
  mf.configure(window);
  std::vector<double> history, w;
  history.reserve(2000);
  rtk::Run run;
  for (int i = 0; i < 2000; ++i)
  {
    double x = std::sin(0.07 * i) + 0.1 * std::sin(1.3 * i);
    if (i % 37 == 0) {x += 5.0;}  // spikes
    history.push_back(x);
    const double y = rtk::guarded(run, [&] { return mf.process(x); });
    const std::size_t n = std::min(history.size(), window);
    w.assign(history.end() - static_cast<long>(n), history.end());
    std::sort(w.begin(), w.end());
    rtk::expect(run, y == w[(n - 1) / 2], "call " + std::to_string(i));
  }
  return rtk::report(run);
}
