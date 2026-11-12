# MovingAverage

Computes the arithmetic mean of a sliding window of samples.

## Public API

### `void configure(size_t window_size)`
Initializes the internal circular buffer. `window_size` must be greater than 0. This method may allocate memory.

### `double process(double sample)`
Processes a new sample, updates the internal sliding window, and returns the current arithmetic mean. 

## Preconditions
- `window_size` must be $\ge 1$.
- The `process` method is called on the real-time path and must not allocate memory, throw, lock, or perform I/O.

## Behavior
- If the buffer has fewer than `window_size` samples (at the beginning), the mean is calculated over the samples received so far.
- Once the buffer is full, the mean is calculated over exactly `window_size` samples.
- The implementation must use a circular buffer to ensure $O(1)$ time complexity per call.
