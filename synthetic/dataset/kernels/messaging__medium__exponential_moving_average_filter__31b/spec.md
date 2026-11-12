# Exponential Moving Average Filter (EMA)

The `ExponentialMovingAverageFilter` provides a real-time smoothing filter for a stream of incoming sensor messages. It implements a first-order infinite impulse response (IIR) filter.

## Mathematical Model
The filtered value $y_n$ is computed as:
$y_n = \alpha \cdot x_n + (1 - \alpha) \cdot y_{n-1}$
where $x_n$ is the current input, $y_{n-1}$ is the previous output, and $\alpha$ is the smoothing factor ($0 < \alpha \le 1$).

## Public API

### `void configure(double alpha)`
- Sets the smoothing factor $\alpha$.
- Precondition: $0.0 < \alpha \le 1.0$.
- This method is called once during initialization and may allocate memory (though not required for this kernel).

### `void push(double value)`
- Integrates a new sample into the filter.
- Updates the internal state.
- Must not allocate heap memory, throw, lock, or perform I/O.

### `double get_value() const`
- Returns the current filtered value.
- For the first sample ever pushed, the filtered value equals that sample.
- Must not allocate heap memory, throw, lock, or perform I/O.

### `void reset()`
- Resets the internal state. The next call to `push` will treat the sample as the first sample.
- Must not allocate heap memory, throw, lock, or perform I/O.

## Real-Time Constraints
The methods `push`, `get_value`, and `reset` are called on a hard real-time path and must adhere to strict non-blocking, non-allocating constraints.
