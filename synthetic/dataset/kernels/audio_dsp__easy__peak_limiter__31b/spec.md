# PeakLimiter

The `PeakLimiter` provides a simple hard-clipping limiter with a fixed look-ahead buffer. It ensures that the output signal never exceeds a specified amplitude threshold by introducing a delay.

## API

### `void configure(float threshold, size_t lookahead_samples)`
Configures the limiter threshold (maximum absolute amplitude) and the look-ahead buffer size. This method may allocate memory.

### `float process(float input)`
Processes a single sample. 
- It pushes the current `input` into the look-ahead buffer.
- It pops the sample from the look-ahead buffer that entered `lookahead_samples` ago.
- If the absolute value of the popped sample exceeds the `threshold`, it is clamped to `threshold * sign(sample)`.
- Otherwise, the popped sample is returned unchanged.

## Constraints
- `process` is a hard real-time method. It must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- The input `lookahead_samples` must be greater than 0.
- The `process` method must be called after `configure`.

## Behavior
The output is a delayed version of the input where any peak exceeding `threshold` is clipped.
