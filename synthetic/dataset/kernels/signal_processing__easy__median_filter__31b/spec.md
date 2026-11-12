# MedianFilter

The `MedianFilter` class implements a sliding-window median filter used to remove impulse noise from a signal.

## Public API

- `void configure(size_t window_size)`
  Configures the internal buffer to hold `window_size` samples.
  **Precondition**: `window_size` must be odd and greater than 0.
  **Note**: This method may allocate memory.

- `float push(float sample)`
  Adds a new sample to the sliding window and returns the median of the current window.
  **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior

- The filter maintains a window of the last `N` samples.
- When a new sample is pushed, the oldest sample in the window is discarded.
- The returned value is the median of the current window.
- Until the window is fully primed (contains `N` samples), the median is calculated based on the samples currently available.
