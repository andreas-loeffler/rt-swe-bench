// Allocating variant: de-interleaves each block into a scratch vector per call.
#pragma once
#include <cstddef>
#include <vector>

class Biquad
{
public:
  void configure(std::size_t channels, double b0, double b1, double b2, double a1, double a2)
  {
    channels_ = channels;
    b0_ = b0; b1_ = b1; b2_ = b2; a1_ = a1; a2_ = a2;
    state_.assign(4 * channels, 0.0);
  }

  void process(const double * in, double * out, std::size_t frames)
  {
    std::vector<double> channel(frames);
    for (std::size_t c = 0; c < channels_; ++c)
    {
      for (std::size_t f = 0; f < frames; ++f) {channel[f] = in[f * channels_ + c];}
      double * s = &state_[4 * c];
      for (std::size_t f = 0; f < frames; ++f)
      {
        const double x = channel[f];
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
