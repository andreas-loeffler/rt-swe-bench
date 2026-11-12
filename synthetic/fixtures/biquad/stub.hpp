#pragma once
#include <cstddef>

class Biquad
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t channels, double b0, double b1, double b2, double a1, double a2)
  {
    (void)channels; (void)b0; (void)b1; (void)b2; (void)a1; (void)a2;
  }

  // Real-time: filter `frames` interleaved frames of `channels` samples.
  void process(const double * in, double * out, std::size_t frames)
  {
    (void)in; (void)out; (void)frames;  // TODO
  }
};
