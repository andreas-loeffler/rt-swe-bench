# Exponential Moving Average (EMA)

The `ExponentialMovingAverage` class implements a first-order Infinite Impulse Response (IIR) filter used for domain estimation and signal smoothing. It computes a weighted average of the current input signal and the previous estimate.

## Public API

### `void configure(float alpha)`
Prepares the filter. 
- `alpha`: The smoothing factor. Must be in the range `[0.0, 1.0]`.
- A value of `1.0` means no smoothing (output equals input).
- A value of `0.0` means the output never changes from the initial state.

### `float process(float input)`
Processes a single sample.
- **Precondition**: `configure()` must be called before the first call to `process()`.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Mathematical Behavior
The output $y[n]$ is computed as:
$y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$

The first sample processed initializes the internal state such that $y[0] = x[0]$.
