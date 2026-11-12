# fir_filter_fixed

Computes a discrete-time Finite Impulse Response (FIR) linear convolution. The filter applies a set of coefficients to an incoming stream of samples.

## Public API

### `void configure(const std::vector<float>& coefficients)`
Sets the filter coefficients. This method may allocate memory to resize internal buffers.

### `float process(float input)`
Processes a single input sample through the FIR filter and returns the output. This method is **real-time safe** (no allocation, no locking, no I/O, no exceptions).

## Preconditions
- `configure` must be called exactly once before any calls to `process`.
- The coefficient vector must not be empty.
- The input to `process` is a single float sample.

## Behavior
- The filter is a standard FIR filter: $y[n] = \sum_{i=0}^{M-1} x[n-i] \cdot h[i]$, where $h$ is the coefficient vector.
- The filter maintains an internal state (delay line) of size equal to the number of coefficients.
- `process` returns the filtered value for each new input.
- The internal buffer is managed as a circular buffer or shift register.
