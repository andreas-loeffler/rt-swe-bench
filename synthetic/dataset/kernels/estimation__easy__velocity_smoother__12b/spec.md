# VelocitySmoother

A real-time filter that smooths raw velocity data using a first-order low-pass filter (exponential moving average).

## API

### `void configure(double alpha)`
Initializes the filter.
- `alpha`: The smoothing factor, where $0 < \alpha \le 1$. 
- A larger $\alpha$ makes the output more responsive to the raw input (less smoothing).
- A smaller $\alpha$ makes the output smoother but with more latency.

### `double process(double raw_velocity)`
Processes a single sample of raw velocity data.
- **Precondition**: `configure` must be called exactly once before any call to `process`.
- **Returns**: The smoothed velocity value.

## Real-Time Constraints
- The `process` method is a real-time call. It must not allocate heap memory, throw exceptions, use mutexes, or perform I/O.
