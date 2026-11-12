# MovingAverageFilter

Computes the arithmetic mean of the last $N$ motion samples to smooth high-frequency jitter.

## Public API
- `void configure(size_t window_size)`
  Initializes the filter with a window size of $N$. This method may allocate memory.
- `double process(double sample)`
  Adds a new sample to the filter and returns the average of the current window.

## Behavior
- The filter maintains a window of the most recent $N$ samples.
- If fewer than $N$ samples have been processed since `configure`, the average is computed over the samples actually received (sum of samples divided by the number of samples processed so far).
- Once $N$ samples have been processed, the oldest sample is discarded as new samples arrive (sliding window).
- All calculations are performed using double-precision floating point.

## Real-time Constraints
- `process` is a hard real-time method.
- It must not allocate heap memory, throw exceptions, perform I/O, or use synchronization primitives (locks).
