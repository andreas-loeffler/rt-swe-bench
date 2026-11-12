# OnePoleFilter

The `OnePoleFilter` implements a first-order exponential smoothing filter (a one-pole low-pass filter) for a signal stream. It uses a recursive difference equation to smooth the input signal.

## API

### `void configure(float sampleRate, float cutoffFreq)`
Configures the filter coefficients based on the provided sample rate and the desired -3dB cutoff frequency. 
- **Preconditions**: `sampleRate > 0` and `0 < cutoffFreq < sampleRate / 2`.
- **Behavior**: Calculates the smoothing factor $\alpha$ used in the recursive equation.

### `float process(float input)`
Processes a single sample through the filter.
- **Behavior**: Updates the internal state based on the recursive formula: $y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$.
- **Returns**: The filtered sample.

### `void reset()`
Resets the internal state (previous output) to zero.
- **Behavior**: Ensures the next call to `process` starts from a zero-initialized state.

## Real-time Constraints
The methods `process` and `reset` are called on a hard real-time path. They must not allocate heap memory, throw exceptions, perform I/O, or use synchronization primitives (locks).
