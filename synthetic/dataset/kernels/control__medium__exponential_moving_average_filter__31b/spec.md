# Exponential Moving Average (EMA) Filter

The `ExponentialMovingAverage` class implements a first-order infinite impulse response (IIR) filter used for smoothing signals. It computes a weighted average of the current input and the previous state.

## API

### `void configure(double alpha)`
Initializes the filter with the smoothing factor $\alpha$.
- **Precondition**: $0.0 \le \alpha \le 1.0$.
- **Behavior**: Sets the weight for the current input. $\alpha = 1.0$ means no smoothing (output = input), and $\alpha = 0.0$ means the output never changes from its initial state.

### `double process(double input)`
Updates the internal state with a new sample and returns the filtered value.
- **Formula**: $y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$
- **Edge Case**: For the first sample processed after `configure`, the output is exactly the input value (initializing the state).
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.

### `double get_state()`
Returns the current internal state (the last computed filtered value) without updating it.
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.

### `void reset()`
Resets the internal state to a default value (0.0) and clears the "first sample" initialization flag.
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.
