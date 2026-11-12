# biquad

Implement `Biquad` in `kernel.hpp`: a second-order IIR filter in Direct Form I,
$y[n] = b_0 x[n] + b_1 x[n-1] + b_2 x[n-2] - a_1 y[n-1] - a_2 y[n-2]$, applied
independently to each channel of an interleaved buffer. Each channel keeps its
own state across calls; the state starts at zero. `configure()` sets the channel
count and the coefficients; `process(in, out, frames)` filters `frames` frames.

`process()` runs on a hard real-time path (an audio callback): it must not
allocate heap memory. `configure()` may.
