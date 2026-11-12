# PointCloudFilter

Applies a distance threshold filter to a fixed-size array of 3D points. If a point's distance from the origin $(0, 0, 0)$ exceeds the specified threshold, it is considered "outside" the range.

## Public API

### `void configure(size_t max_points, float threshold)`
Configures the filter with the maximum number of points to process and the distance threshold.
- **Preconditions**: `max_points > 0`, `threshold >= 0.0f`.
- **Allocations**: May allocate memory for the point buffer.

### `size_t process(const float* input_points, size_t count, float* output_points)`
Processes a buffer of 3D points. The input buffer `input_points` contains flattened 3D coordinates $(x, y, z, x, y, z, \dots)$.
- **Input**: 
    - `input_points`: Pointer to a buffer of `count * 3` floats.
    - `count`: Number of 3D points.
- **Output**: 
    - `output_points`: Pointer to a buffer of `count * 3` floats.
    - Returns the number of points that passed the filter (distance $\le$ threshold).
- **Behavior**: 
    - If `count > max_points`, the first `max_points` elements are processed.
    - Points that satisfy $x^2 + y^2 + z^2 \le \text{threshold}^2$ are copied to `output_points`.
    - Points that fail are omitted from `output_points`.
- **Real-time Constraints**: Must not allocate, throw, lock, or perform I/O.

## Behavior
- If the input count is 0, the function returns 0.
- If no points satisfy the threshold, the function returns 0 and `output_points` is left unchanged.
- The filter uses a squared distance comparison to avoid `sqrt` overhead.
