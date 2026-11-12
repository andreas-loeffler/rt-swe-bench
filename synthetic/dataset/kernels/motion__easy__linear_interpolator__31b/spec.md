# LinearInterpolator

Computes the current position based on a linear interpolation between a start position and an end position over a fixed duration (measured in number of samples).

## API

- `void configure(float start_pos, float end_pos, size_t duration_samples)`
  Configures the interpolation parameters. This is called once before the real-time path.
- `float step()`
  Advances the internal interpolation clock by one sample and returns the interpolated position.

## Behavior

- The interpolation starts at `start_pos` and reaches `end_pos` exactly at the sample index `duration_samples`.
- If `duration_samples` is 0, `step()` always returns `end_pos`.
- After `duration_samples` have been processed, the output clamps to `end_pos`.
- The interpolation is computed as: `current_pos = start_pos + (end_pos - start_pos) * (current_sample / duration_samples)`.

## Real-time Constraints

- `step()` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
