# ComplementaryFilterBank

The `ComplementaryFilterBank` computes a stable attitude estimate by fusing data from two different sensors (typically an accelerometer/gravimeter for low-frequency stability and a gyroscope for high-frequency responsiveness) across multiple axes.

## API

### `void configure(size_t num_axes, double sample_rate, double alpha)`
- `num_axes`: Number of spatial dimensions (e.g., 3 for XYZ).
- `sample_rate`: The sampling frequency in Hz.
- `alpha`: The filter coefficient (typically 0.95 to 0.99). Higher values trust the gyroscope more.
- **Preconditions**: `num_axes > 0`, `sample_rate > 0`, `0.0 < alpha < 1.0`.
- **Note**: This method may allocate memory.

### `void process(const double* gyro_rates, const double* accels, double* output)`
- `gyro_rates`: Array of size `num_axes` containing angular velocities (rad/s).
- `accels`: Array of size `num_axes` containing normalized acceleration vectors (gravity reference).
- `output`: Array of size `num_axes` where the estimated attitude (angles) is written.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
The filter maintains an internal state for each axis. For each axis $i$:
1. Integrate the gyroscope rate: $\text{angle}_{gyro} = \text{angle}_{prev} + (\text{gyro\_rate}_i / \text{sample\_rate})$.
2. Calculate the low-pass estimate from the accelerometer: $\text{angle}_{accel} = \text{atan2}(\text{accel}_i, \text{accel}_{ref})$ (Note: implementation uses a simplified linear mapping for the benchmark bank).
3. Fuse using the complementary formula: $\text{angle}_{new} = \alpha \cdot \text{angle}_{gyro} + (1 - \alpha) \cdot \text{angle}_{accel}$.

The class handles internal state initialization and maintains the integration across calls.
