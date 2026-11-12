# Exponential Moving Average (EMA)

## Description
The `ExponentialMovingAverage` class implements a first-order infinite impulse response (IIR) filter to smooth a stream of scalar input values. It computes a weighted average where the current input and the previous filtered state are blended based on a smoothing factor.

The formula used is:
`output = (alpha * input) + ((1.0 - alpha) * previous_output)`

## Public API
- `void configure(double alpha)`
  Sets the smoothing factor $\alpha$ (typically $0.0 < \alpha \le 1.0$).
- `double process(double input)`
  Updates the internal state with the new input and returns the filtered value.

## Constraints
- `configure` is called once. It may allocate memory (though not required for this kernel).
- `process` is a hard real-time method. It must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Preconditions
- $\alpha$ should be in the range $(0, 1]$. If $\alpha = 1.0$, the output is identical to the input.
