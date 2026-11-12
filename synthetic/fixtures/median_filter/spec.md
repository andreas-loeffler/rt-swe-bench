# median_filter

Implement `MedianFilter` in `kernel.hpp`: `process(x)` returns the lower median
of the last `window` samples (of all samples so far before `window` have
arrived), that is, element (n - 1) / 2 of the n samples in sorted order.
`configure(window)` is called once before the first `process()`; `window` is at
least 1.

`process()` runs on a hard real-time path: it must not allocate heap memory.
`configure()` may.
