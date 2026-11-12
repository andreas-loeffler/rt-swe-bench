# BiquadFilter Spec

The `BiquadFilter` class implements a second-order Infinite Impulse Response (IIR) filter using a Direct Form II implementation. It allows for processing a stream of floating-point samples to achieve low-pass, high-pass, or band-pass filtering based on the coefficients provided.

## Public API

### `void configure(float b0, float b1, float b2, float a1, float a2)`
Initializes the filter coefficients.
- `b0, b1, b2`: Feed-forward coefficients.
- `a1, a2`: Feed-back coefficients (where the transfer function denominator is $1 + a1 \cdot z^{-1} + a2 \cdot z^{-2}$).
- This method is called once before the real-time path and may perform initialization.

### `float process(float input)`
Processes a single sample through the filter.
- **Input**: A single floating-point sample.
- **Output**: The filtered output sample.
- **Constraint**: This method is executed on a hard real-time path. It must not allocate heap memory, throw exceptions, perform I/O, or acquire locks.

### `void reset()`
Resets the internal filter state (delay registers) to zero.
- **Constraint**: This method is executed on a hard real-time path. It must not allocate heap memory, throw exceptions, perform I/O, or acquire locks.

## Behavior
The filter implements the difference equation:
$w[n] = x[n] - a1 \cdot w[n-1] - a2 \cdot w[n-2]$
$y[n] = b0 \cdot w[n] + b1 \cdot w[n-1] + b2 \cdot w[n-2]$

where $x$ is the input, $y$ is the output, and $w$ is the internal state.
