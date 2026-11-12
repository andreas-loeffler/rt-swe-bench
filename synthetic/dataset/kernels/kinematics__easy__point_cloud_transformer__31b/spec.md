# PointCloudTransformer

The `PointCloudTransformer` kernel transforms a batch of 3D points from a sensor-local coordinate frame to a global world frame using a 4x4 homogeneous transformation matrix.

## Public API

### `void configure(size_t max_points)`
- **Purpose**: Pre-allocates memory for the point cloud processing.
- **Preconditions**: `max_points` must be greater than 0.

### `void transform(const float* input_points, float* output_points, const float* transform_matrix, size_t num_points)`
- **Purpose**: Applies the transformation matrix to the input points.
- **Input**: 
    - `input_points`: Pointer to an array of `num_points * 3` floats (x, y, z).
    - `output_points`: Pointer to an array of `num_points * 3` floats where results are stored.
    - `transform_matrix`: Pointer to 16 floats representing a 4x4 homogeneous transformation matrix in row-major order.
    - `num_points`: Number of points to transform.
- **Preconditions**: 
    - `num_points` must be $\le$ `max_points` configured.
    - Pointers must be valid and non-null.
- **Real-time Guarantee**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Mathematical Operation
For each point $P = [x, y, z, 1]^T$:
$P_{world} = M_{transform} \times P$
The resulting $x, y, z$ components are written to `output_points`.
