# TransformInterpolator

The `TransformInterpolator` computes linear interpolations between two 3D transformation matrices across a fixed number of discrete sub-steps. 

## Public API

### `void configure(size_t num_steps)`
Prepares the interpolator for a sequence of `num_steps` linear interpolations. This method may allocate memory.

### `void interpolate(const float start[16], const float end[16], size_t step, float out[16])`
Computes the interpolated matrix at a specific step index.
- **Preconditions**: `configure` must have been called. `step` must be in the range `[0, num_steps - 1]`.
- **Behavior**: For each element $i \in [0, 15]$, the output is calculated as:
  $out[i] = start[i] + (end[i] - start[i]) \times \frac{step}{num\_steps - 1}$
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
