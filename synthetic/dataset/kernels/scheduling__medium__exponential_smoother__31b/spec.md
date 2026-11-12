# ExponentialSmoother

The `ExponentialSmoother` implements a first-order infinite impulse response (IIR) low-pass filter to remove high-frequency noise from a stream of sensor readings.

## API

### `void configure(double alpha, double initial_value)`
Configures the smoother.
- `alpha`: The smoothing factor $\alpha \in (0, 1]$. Higher values give more weight to new data (less smoothing).
- `initial_value`: The starting value of the filter state.

### `double push(double input)`
Processes a new sample and returns the updated smoothed value.
- **Behavior**: $y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$.
- **Precondition**: `configure` must be called before the first `push`.

### `double get_current()` const
Returns the current smoothed value without updating the state.

## Real-Time Requirements
The methods `push` and `get_current` run on a hard real-time path. They must not:
- Allocate or free heap memory.
- Throw exceptions.
- Perform I/O operations.
- Use mutexes or other blocking synchronization primitives.
