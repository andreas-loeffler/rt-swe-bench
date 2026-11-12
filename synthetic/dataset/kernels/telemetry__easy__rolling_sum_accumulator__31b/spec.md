# RollingSumAccumulator

Maintains a running sum of telemetry samples over a fixed-length sliding window.

## Public API
- `void configure(size_t window_size)`: Sets the maximum number of samples the window can hold. Allocates necessary memory.
- `double push(double sample)`: Adds a new sample to the window. If the window was already full, the oldest sample is removed. Returns the current sum of all samples in the window.

## Preconditions
- `configure` must be called before any call to `push`.
- `window_size` must be greater than 0.

## Real-Time Constraints
The `push` method is executed on a hard real-time path. It must not:
- Allocate or free heap memory.
- Throw exceptions.
- Use mutexes or locks.
- Perform any I/O operations.
