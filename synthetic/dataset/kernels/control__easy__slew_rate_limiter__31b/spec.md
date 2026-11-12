# Slew Rate Limiter

The `SlewRateLimiter` constrains the rate of change of a signal. If the difference between the current target value and the previous output exceeds a defined maximum change per sample, the output is clamped to the maximum allowable change in the direction of the target.

## API
- `void configure(double max_delta, double initial_value)`: Sets the maximum allowable change per sample and initializes the starting state of the filter.
- `double process(double input)`: Processes a single sample. Returns the slewed output.

## Constraints
- `process` is a hard real-time method. It must not allocate heap memory, throw exceptions, perform I/O, or use locks.
- `configure` may allocate memory.

## Behavior
Given a target `input` and the previous output `prev`:
1. Calculate `diff = input - prev`.
2. If `diff > max_delta`, the result is `prev + max_delta`.
3. If `diff < -max_delta`, the result is `prev - max_delta`.
4. Otherwise, the result is `input`.
