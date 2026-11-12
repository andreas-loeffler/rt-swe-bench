# VelocityProfileLimiter

The `VelocityProfileLimiter` is a stateful saturation filter used in robot kinematics to ensure that commanded joint velocities do not violate physical hardware limits on maximum velocity and maximum acceleration.

## Public API

### `void configure(size_t num_joints, double max_vel, double max_accel, double dt)`
- **Purpose**: Initializes the limiter for a specific number of joints and sets the physical constraints.
- **Parameters**:
    - `num_joints`: Number of degrees of freedom to track.
    - `max_vel`: Maximum absolute velocity ($\text{rad/s}$ or $\text{m/s}$).
    - `max_accel`: Maximum absolute acceleration ($\text{rad/s}^2$ or $\text{m/s}^2$).
    - `dt`: The fixed sampling time interval ($\text{s}$).
- **Behavior**: Allocates necessary memory for internal state.

### `void process(const double* desired_vel, double* actual_vel)`
- **Purpose**: Clamps the desired velocity based on current state and physical limits.
- **Parameters**:
    - `desired_vel`: Pointer to an array of size `num_joints` containing requested velocities.
    - `actual_vel`: Pointer to an array of size `num_joints` where the limited velocities will be written.
- **Real-time Constraints**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- **Logic**:
    1. For each joint, calculate the maximum allowable velocity change ($\Delta v = \text{max\_accel} \times dt$).
    2. The output velocity must be within $[\text{current\_vel} - \Delta v, \text{current\_vel} + \Delta v]$.
    3. The output velocity must also be within $[-\text{max\_vel}, \text{max\_vel}]$.
    4. The state (current velocity) is updated to the output velocity for the next iteration.
