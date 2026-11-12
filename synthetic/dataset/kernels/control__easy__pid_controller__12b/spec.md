# PidController

The `PidController` class implements a standard Proportional-Integral-Derivative (PID) control loop. It computes a control output based on the current process variable (PV) and a setpoint (SP).

## API

### `void configure(double Kp, double Ki, double Kd, double dt)`
Initializes the controller parameters:
- `Kp`: Proportional gain.
- `Ki`: Integral gain.
- `Kd`: Derivative gain.
- `dt`: The sampling time interval (must be positive).

### `double process(double setpoint, double current_value)`
Computes the control output. The output is calculated as:
- `error = setpoint - current_value`
- `integral = integral + error * dt`
- `derivative = (error - previous_error) / dt`
- `output = (Kp * error) + (Ki * integral) + (Kd * derivative)`

The `previous_error` is updated internally at the end of each `process` call.

## Constraints
- The `process` method is real-time and must not allocate, throw, lock, or perform I/O.
- The `dt` provided in `configure` must be greater than zero.
