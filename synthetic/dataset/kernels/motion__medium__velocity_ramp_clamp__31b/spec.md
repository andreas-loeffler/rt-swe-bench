# VelocityRampClamp

The `VelocityRampClamp` kernel implements a first-order velocity ramp with hard saturation limits. It smoothly transitions the current velocity toward a target setpoint while ensuring the velocity never exceeds defined bounds and the rate of change (acceleration) is capped.

## API

### `void configure(double max_vel, double max_accel, double dt)`
Initializes the kernel parameters.
- `max_vel`: The absolute maximum allowable velocity (saturates at $\pm \text{max\_vel}$).
- `max_accel`: The maximum allowable change in velocity per step (ramp rate).
- `dt`: The constant time step for the ramp calculation.
- **Preconditions**: `max_vel > 0`, `max_accel > 0`, `dt > 0`.

### `double step(double target_vel)`
Updates the internal state and returns the new current velocity.
- **Behavior**:
    1. Clamps the `target_vel` to the range `[-max_vel, max_vel]`.
    2. Calculates the difference between the clamped target and the current velocity.
    3. Limits this difference to a maximum of `max_accel * dt` (maintaining the sign).
    4. Updates the current velocity by adding this limited delta.
    5. Returns the resulting velocity.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `double get_velocity() const`
Returns the current velocity without modifying the state.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
