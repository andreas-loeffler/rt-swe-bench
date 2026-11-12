# CascadedPIDController Specification

The `CascadedPIDController` implements a nested control loop structure typically used in robotics (e.g., a position loop providing the setpoint for a velocity loop).

## Mathematical Behavior
- **Outer Loop**: Computes a target for the inner loop based on the error between a primary setpoint and the primary measurement.
- **Inner Loop**: Computes the final actuator output based on the error between the target provided by the outer loop and the secondary measurement.
- Both loops use a Parallel PID form: $u(t) = K_p e(t) + K_i \int e(t)dt + K_d \frac{de(t)}{dt}$.
- The controller supports an "Anti-Windup" clamp on the integrator and a hard output limit.
- The controller operates in two modes: `STANDBY` (output 0, integrals reset) and `ACTIVE` (full control).

## Public API

### Configuration
- `void configure(double dt, double outer_kp, double outer_ki, double outer_kd, double inner_kp, double inner_ki, double inner_kd, double out_min, double out_max)`
  - Called once before real-time execution.
  - Sets the sampling period and PID gains for both loops.
  - Defines the hard limits for the final actuator output.

### Real-time Methods
The following methods must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

- `void set_mode(bool active)`
  - Switches between `STANDBY` and `ACTIVE`. 
  - Transitioning to `STANDBY` resets the internal integrators of both loops.
- `double step(double setpoint, double outer_meas, double inner_meas)`
  - Computes the control signal.
  - `setpoint`: Desired value for the outer loop.
  - `outer_meas`: Current measured value for the outer loop.
  - `inner_meas`: Current measured value for the inner loop.
  - Returns the clamped actuator output.

## Preconditions
- `configure` must be called before `step` or `set_mode`.
- `dt` must be greater than zero.
- `out_max` must be greater than `out_min`.
