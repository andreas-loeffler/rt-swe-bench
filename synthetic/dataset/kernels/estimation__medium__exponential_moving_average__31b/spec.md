# ExponentialMovingAverage

The `ExponentialMovingAverage` class implements a first-order recursive low-pass filter (Exponential Moving Average) used for domain estimation and noise reduction in real-time data streams.

## Public API

### `void configure(double alpha)`
Configures the filter smoothing factor.
- **`alpha`**: The smoothing factor $\alpha \in (0, 1]$. A value closer to 1.0 gives more weight to the new sample (less smoothing), while a value closer to 0.0 gives more weight to the history (more smoothing).
- This method is called once and may allocate memory.

### `void push(double sample)`
Feeds a new sample into the filter to update the internal state.
- Updates the moving average: $S_t = \alpha \cdot x_t + (1 - \alpha) \cdot S_{t-1}$.
- If the filter has not been initialized with a sample, the first sample becomes the initial state.

### `double pop()`
Returns the current smoothed value of the filter.
- Returns the result of the most recent `push` operation.
- If `pop` is called before any `push`, it returns $0.0$.

## Real-Time Constraints
The methods `push` and `pop` are hard real-time:
- They must not allocate heap memory.
- They must not throw exceptions.
- They must not use locks/mutexes.
- They must not perform I/O.
