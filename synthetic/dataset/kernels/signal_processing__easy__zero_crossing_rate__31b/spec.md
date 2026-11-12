# ZeroCrossingRate Kernel

## Description
The `ZeroCrossingRate` kernel computes the number of times a discrete-time signal crosses zero within a sliding window of length `N`. A zero-crossing is defined as occurring when two consecutive samples in the signal have opposite signs.

## API
- `void configure(size_t window_size)`: Initializes the internal buffer to hold `window_size` samples. Must be called before any call to `push`.
- `float push(float sample)`: Pushes a new sample into the sliding window and returns the current Zero Crossing Rate (ZCR).
  - ZCR is calculated as: `(number of zero crossings in window) / (window_size - 1)`.
  - If the window is not yet full, the ZCR is calculated based on the number of samples currently available.

## Preconditions
- `configure` must be called with `window_size > 1`.
- `push` must be called after `configure`.

## Real-time Constraints
The method `push` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O. All necessary memory must be allocated during the `configure` call.
