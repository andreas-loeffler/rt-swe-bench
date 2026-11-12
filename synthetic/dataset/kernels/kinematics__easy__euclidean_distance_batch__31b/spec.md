# DistanceCalculator

## Description
The `DistanceCalculator` computes the Euclidean distance between a single reference point (the observer) and a batch of target coordinates in 3D space.

## API
- `void configure(size_t num_targets)`: Pre-allocates internal buffers to hold coordinates for `num_targets` points.
- `void set_targets(const float* coords)`: Copies `num_targets * 3` floats from the provided pointer into the internal buffer.
- `void process(const float ref_point[3], float* distances)`: Computes the distance from `ref_point` to each of the cached targets and stores the results in the `distances` array.

## Preconditions
- `configure` must be called before `set_targets` or `process`.
- `set_targets` must be called before `process` to ensure targets are initialized.
- In `set_targets`, `coords` must point to at least `num_targets * 3` valid floats.
- In `process`, `ref_point` must point to 3 floats, and `distances` must point to at least `num_targets` floats.

## Real-time Constraints
The methods `set_targets` and `process` are executed on a hard real-time path. They must not perform heap allocations, throw exceptions, acquire locks, or perform I/O operations.
