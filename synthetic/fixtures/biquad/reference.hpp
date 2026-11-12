// Reference: Direct Form I biquad, per-channel state sized in configure().
#pragma once
#include <cstddef>
#include <vector>

class Biquad
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t channels, double b0, double b1, double b2, double a1, double a2)
  {
    channels_ = channels;
    b0_ = b0; b1_ = b1; b2_ = b2; a1_ = a1; a2_ = a2;
    state_.assign(4 * channels, 0.0);  // x1, x2, y1, y2 per channel
  }

  // Real-time: filter `frames` interleaved frames of `channels` samples.
  void process(const double * in, double * out, std::size_t frames)
  {
    for (std::size_t f = 0; f < frames; ++f)
    {
      for (std::size_t c = 0; c < channels_; ++c)
      {
        double * s = &state_[4 * c];
        const double x = in[f * channels_ + c];
        const double y = b0_ * x + b1_ * s[0] + b2_ * s[1] - a1_ * s[2] - a2_ * s[3];
        s[1] = s[0]; s[0] = x; s[3] = s[2]; s[2] = y;
        out[f * channels_ + c] = y;
      }
    }
  }

private:
  std::size_t channels_ = 1;
  double b0_ = 1, b1_ = 0, b2_ = 0, a1_ = 0, a2_ = 0;
  std::vector<double> state_;
};
