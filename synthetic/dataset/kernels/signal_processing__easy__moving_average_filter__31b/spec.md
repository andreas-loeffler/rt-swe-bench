# MovingAverageFilter

Computes the arithmetic mean of the last $N$ samples of a signal.

## API
- `void configure(size_t window_size)`: Sets the window size $N$. Allocates necessary memory.
- `double process(double sample)`: Pushes a new sample into the filter and returns the average of the last $N$ samples.

## Behavior
- If fewer than $N$ samples have been processed since `configure`, the filter computes the average of all samples received so far.
- Once $N$ samples have been processed, the oldest sample is discarded as new samples arrive (sliding window).
- The filter must handle the transition from the filling phase to the full window phase.

## Real-time Constraints
- The `process` method is called on a hard real-time path.
- `process` must not allocate heap memory, throw exceptions, acquire locks, or perform I/O.
