# SlewLimiter Specification

The `SlewLimiter` prevents rapid transitions in a signal by limiting the maximum change allowed per processing cycle. This is essential for preventing actuator saturation in robotics and audible pops in audio synthesis.

## API

### `void configure(double max_delta, double initial_value)`
Configures the limiter.
- `max_delta`: The maximum absolute change allowed per call to `process`.
- `initial_value`: The starting value of the signal.

### `double process(double target)`
Updates the current internal state toward the `target` value and returns the result.
- If `|target - current| <= max_delta`, the result is `target`.
- If `target > current + max_delta`, the result is `current + max_delta`.
- If `target < current - max_delta`, the result is `current - max_delta`.

### `double get_current() const`
Returns the current state of the signal without modifying it.

## Constraints
- `configure` may allocate memory (though none is required for this specific implementation).
- `process` and `get_current` are **hard real-time**: they must not allocate heap memory, throw exceptions, use locks, or perform I/O.
