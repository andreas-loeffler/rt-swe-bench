# ExponentialSmoother

The `ExponentialSmoother` implements a first-order Infinite Impulse Response (IIR) low-pass filter. It is used to reduce high-frequency noise or jitter from a signal.

## Mathematical Formula
The smoothed value at step $n$ is calculated as:
$y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$
where:
- $x[n]$ is the current input sample.
- $y[n-1]$ is the previous output sample.
- $\alpha$ is the smoothing factor (alpha) in the range $[0, 1]$.

For the first sample processed after configuration, $y[n-1]$ is initialized to the first input $x[0]$.

## Public API

### `void configure(double alpha)`
Configures the smoother with the smoothing factor $\alpha$.
- **Precondition**: $0.0 \le \alpha \le 1.0$.
- **Side effect**: Resets the internal state.

### `double process(double input)`
Processes a single sample and returns the smoothed value.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
- If $\alpha = 1.0$, the output is identical to the input (no smoothing).
- If $\alpha = 0.0$, the output remains constant at the value of the first sample processed.
