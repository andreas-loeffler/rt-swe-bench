# AlphaBetaFilter

Computes a smoothed estimate of a signal using a time-varying weighted average of the previous state and new measurements. This is a first-order recursive filter where the smoothing factor $\alpha$ determines the influence of the current measurement.

## API

### `void configure(double initial_alpha, double initial_value)`
Prepares the filter for real-time execution.
- `initial_alpha`: The starting smoothing factor $\alpha \in [0, 1]$.
- `initial_value`: The starting state of the filter.

### `double process(double measurement, double alpha)`
Processes a new measurement and returns the updated smoothed estimate.
- `measurement`: The new raw data point.
- `alpha`: The smoothing factor to use for this specific step $\in [0, 1]$.

**Real-time constraints:**
- `process` must not allocate heap memory, throw exceptions, use locks, or perform I/O.
