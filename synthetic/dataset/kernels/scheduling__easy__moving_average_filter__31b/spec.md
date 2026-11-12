# MovingAverageFilter

The `MovingAverageFilter` computes the arithmetic mean of the last $N$ samples provided to it.

## Public API

- `void configure(size_t window_size)`
  Initializes the filter to maintain a window of $N$ samples.
- `double process(double input)`
  Adds the `input` sample to the window and returns the current mean of all samples currently stored in the buffer.

## Behavior

1. **Windowing**: The filter maintains a circular buffer of size $N$.
2. **Warm-up Phase**: Until $N$ samples have been processed, the mean is calculated based on the number of samples actually received (i.e., if 3 samples have been pushed into a window of size 10, the mean is the sum of those 3 divided by 3).
3. **Steady State**: Once $N$ samples have been processed, the oldest sample is replaced by the newest, and the mean is always the sum of the last $N$ samples divided by $N$.
4. **Real-time Constraints**: The `process` method must not allocate heap memory, throw exceptions, use locks, or perform I/O.

## Preconditions

- `configure` must be called exactly once before any call to `process`.
- `window_size` must be greater than 0.
