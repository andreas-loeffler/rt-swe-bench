#pragma once
#include <cstddef>

class MedianFilter
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t window) { (void)window; }

  // Real-time: lower median of the last `window` samples (of all so far before
  // the window is full): element (n - 1) / 2 of the n samples in sorted order.
  double process(double x) { (void)x; return 0.0; }  // TODO
};
