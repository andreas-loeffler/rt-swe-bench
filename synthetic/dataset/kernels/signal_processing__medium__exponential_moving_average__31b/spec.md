# ExponentialMovingAverage

The `ExponentialMovingAverage` class implements a first-order IIR (Infinite Impulse Response) low-pass filter. It smooths an incoming stream of scalar values based on a configurable time constant.

## Mathematical Model
The filter follows the recurrence relation:
`y[n] = α * x[n] + (1 - α) * y[n-1]`

Where:
- `x[n]` is the current input.
- `y[n]` is the current filtered output.
- `α` (alpha) is the smoothing factor derived from the sample rate and time constant $\tau$:
  `α = 1 - exp(-1 / (sample_rate * τ))`

## Public API

### `void configure(double sample_rate, double time_constant)`
Prepares the filter for operation. 
- `sample_rate`: The frequency of incoming samples in Hz.
- `time_constant`: The time constant $\tau$ in seconds.
- **Preconditions**: `sample_rate > 0` and `time_constant > 0`.
- This method may allocate memory.

### `double step(double input)`
Processes a single input sample and returns the updated filtered value.
- **Real-time constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `double get_current_value()`
Returns the most recently computed filtered value without updating the internal state.
- **Real-time constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `void reset()`
Resets the internal state (the previous output value) to zero.
- **Real-time constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
