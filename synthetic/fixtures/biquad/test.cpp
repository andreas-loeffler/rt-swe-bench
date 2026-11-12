// Tier 1 test: biquad. 300 blocks of 64 stereo frames through a low-pass biquad,
// checked sample by sample against an independent Direct Form I computation.
#include <cmath>
#include <string>
#include <vector>

#include "kernel.hpp"
#include "rtk/check.hpp"

int main()
{
  rtk::setup();
  const std::size_t channels = 2, frames = 64;
  const double b0 = 0.0675, b1 = 0.1349, b2 = 0.0675, a1 = -1.1430, a2 = 0.4128;
  Biquad bq;
  bq.configure(channels, b0, b1, b2, a1, a2);
  std::vector<double> in(channels * frames), out(channels * frames);
  std::vector<double> s(4 * channels, 0.0);  // independent state
  rtk::Run run;
  std::size_t n = 0;
  for (int block = 0; block < 300; ++block)
  {
    for (std::size_t f = 0; f < frames; ++f, ++n)
    {
      in[f * channels] = std::sin(0.05 * n);
      in[f * channels + 1] = std::cos(0.3 * n) * 0.5;
    }
    rtk::guarded_void(run, [&] { bq.process(in.data(), out.data(), frames); });
    for (std::size_t f = 0; f < frames; ++f)
    {
      for (std::size_t c = 0; c < channels; ++c)
      {
        double * st = &s[4 * c];
        const double x = in[f * channels + c];
        const double y = b0 * x + b1 * st[0] + b2 * st[1] - a1 * st[2] - a2 * st[3];
        st[1] = st[0]; st[0] = x; st[3] = st[2]; st[2] = y;
        rtk::expect(run, rtk::near(out[f * channels + c], y, 1e-12),
                    "block " + std::to_string(block) + " frame " + std::to_string(f));
      }
    }
  }
  return rtk::report(run);
}
