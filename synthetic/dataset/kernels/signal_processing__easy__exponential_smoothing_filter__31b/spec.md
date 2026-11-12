# ExponentialSmoothingFilter

## Description
The `ExponentialSmoothingFilter` implements a first-order Infinite Impulse Response (IIR) low-pass filter. It smooths a sequence of input samples by maintaining an internal state of the filtered signal. 

The formula for the filter is:
`y[n] = alpha * x[n] + (1 - alpha) * y[n-1]`

Where:
- `x[n]` is the current input sample.
- `y[n]` is the current output sample.
- `y[n-1]` is the previous output sample.
- `alpha` is the smoothing factor ($0.0 \le \alpha \le 1.0$).

## Public API

### `void configure(float alpha)`
Configures the filter with the smoothing factor `alpha`.
- **Precondition**: $0.0 \le \alpha \le 1.0$.
- **Real-time properties**: May allocate memory (though not required for this specific kernel).

### `float process(float input)`
Processes a single input sample and returns the filtered result.
- **Real-time properties**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
- If $\alpha = 1.0$, the filter is bypassed (output equals input).
- If $\alpha = 0.0$, the filter holds the initial state (output remains constant).
- The filter is initialized to an internal state of $0.0$ until the first sample is processed or specifically configured.
