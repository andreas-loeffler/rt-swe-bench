# ImpulseResponseConvolver

## Description
Performs a time-domain convolution of a single-channel input signal with a pre-configured finite impulse response (FIR) filter. The filter is applied as a sliding window over the input stream, computing the sum of products of the current and previous input samples and the filter coefficients.

## Public API
- `void configure(std::span<const float> coefficients)`
  Configures the filter coefficients. This method is called once before the real-time path and may allocate memory to store the filter state.
- `float process(float input)`
  Processes a single sample. Returns the filtered output sample.

## Preconditions
- `configure` must be called before any call to `process`.
- The coefficients span must not be empty.

## Real-time Requirements
- The `process` method must be execution-time deterministic.
- The `process` method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
