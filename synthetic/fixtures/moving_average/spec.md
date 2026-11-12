# moving_average

Implement `MovingAverage` in `kernel.hpp`. `process(x)` is called once per control
cycle and returns the mean of the last `window` samples passed to it (of all
samples so far while fewer than `window` have arrived). `configure(window)` is
called once before the first `process()` call; `window` is at least 1.

`process()` runs on a hard real-time path: it must not allocate heap memory.
`configure()` may.
