# WheelOdometryIntegrator

The `WheelOdometryIntegrator` transforms raw differential wheel encoder ticks into a global 2D pose (x, y, theta). It employs a fixed-size history buffer to smooth the linear and angular velocities before integrating them into the pose.

## Public API

### `void configure(double wheel_base, double ticks_per_meter, size_t window_size)`
Called once during initialization. 
- `wheel_base`: Distance between the two wheels (meters).
- `ticks_per_meter`: Scaling factor to convert encoder ticks to distance.
- `window_size`: Number of samples used for the moving average smoothing.

### `void push_ticks(int32_t left_ticks, int32_t right_ticks, double dt)`
Updates the internal velocity history and integrates the pose.
- `left_ticks`, `right_ticks`: Cumulative encoder ticks since the last call (relative delta).
- `dt`: Time delta since last call (seconds).
- **Behavior**:
    1. Calculates raw linear velocity $v$ and angular velocity $\omega$ from the tick deltas.
    2. Pushes these values into a ring buffer of size `window_size`.
    3. Computes the smoothed velocity $\bar{v}$ and $\bar{\omega}$ as the average of the current buffer contents.
    4. Integrates the smoothed values into the global pose:
       - $x_{new} = x_{old} + \bar{v} \cdot \cos(\theta + \bar{\omega} \cdot dt / 2) \cdot dt$
       - $y_{new} = y_{old} + \bar{v} \cdot \sin(\theta + \bar{\omega} \cdot dt / 2) \cdot dt$
       - $\theta_{new} = \theta_{old} + \bar{\omega} \cdot dt$
    5. Normalizes $\theta$ to $[-\pi, \pi)$.

### `Pose get_pose() const`
Returns the current integrated global pose.
- Returns a `Pose` struct containing `{x, y, theta}`.

## Constraints
- The methods `push_ticks` and `get_pose` are executed on a hard real-time path.
- They must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- All memory must be reserved during `configure`.
