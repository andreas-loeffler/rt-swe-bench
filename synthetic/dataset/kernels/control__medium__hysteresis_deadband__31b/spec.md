# HysteresisDeadband Kernel

The `HysteresisDeadband` kernel filters a continuous signal to eliminate chatter/oscillations around a zero point. Unlike a simple deadband (which returns 0 within a range), a hysteresis deadband requires the signal to cross a specific "activation" threshold to change state, and stay beyond a "release" threshold to return to zero.

## API

### `void configure(double activate_threshold, double release_threshold)`
Configures the thresholds for the filter.
- `activate_threshold`: The absolute value the input must exceed to trigger a non-zero output.
- `release_threshold`: The absolute value the input must fall below to return the output to zero.
- Precondition: `activate_threshold >= release_threshold` and both must be non-negative.

### `double process(double input)`
Processes a single sample of the signal.
- If the current state is `Zero`:
    - If `abs(input) >= activate_threshold`, the state becomes `Active` and the output is `input`.
    - Otherwise, the state remains `Zero` and the output is `0.0`.
- If the current state is `Active`:
    - If `abs(input) <= release_threshold`, the state becomes `Zero` and the output is `0.0`.
    - Otherwise, the state remains `Active` and the output is `input`.

## Real-time Constraints
The `process` method is executed on a hard real-time path. It must not allocate heap memory, throw exceptions, perform I/O, or acquire locks.
