#pragma once
#include <cstddef>

class MovingAverage
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t window) { (void)window; }

  // Real-time: mean of the last `window` samples (fewer before the window is full).
  double process(double x)
  {
    (void)x;
    return 0.0;  // TODO
  }
};
