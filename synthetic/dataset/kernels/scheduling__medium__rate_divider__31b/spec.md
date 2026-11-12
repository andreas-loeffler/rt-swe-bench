# RateDivider

The `RateDivider` kernel transforms a high-frequency master clock trigger into a lower-frequency trigger. It ensures that a target frequency is met by tracking accumulated time (in samples/cycles) and triggering a signal when the accumulation reaches a calculated threshold.

## Public API

### `void configure(double master_rate, double target_rate)`
Sets up the divider. 
- `master_rate`: The frequency of the calling process (e.g., 48000 Hz).
- `target_rate`: The desired output frequency (e.g., 1000 Hz).
- Precondition: `master_rate > 0` and `target_rate > 0`.
- Precondition: `target_rate <= master_rate`.

### `bool process()`
Called at every cycle of the master clock.
- Returns `true` if the target rate interval has elapsed, triggering the divider.
- Returns `false` otherwise.
- This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `double get_phase()`
Returns the current normalized phase of the divider.
- Returns a value in the range `[0.0, 1.0)`, where `0.0` is the start of a cycle and `1.0` is the point of trigger.
- This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
The divider uses an accumulator. For every `process()` call, the accumulator increases by `target_rate / master_rate`. When the accumulator reaches or exceeds `1.0`, it wraps around (subtracts `1.0`) and returns `true`. This handles non-integer ratios by preserving the remainder, ensuring the average frequency over time is exactly the `target_rate`.
