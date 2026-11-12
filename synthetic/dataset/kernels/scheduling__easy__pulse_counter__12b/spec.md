# PulseCounter

The `PulseCounter` kernel tracks the number of rising edges of a digital signal and provides a mechanism to track how many times a specific threshold has been exceeded.

## API

### `void configure(size_t max_threshold)`
Initializes the kernel. 
- `max_threshold`: The maximum value at which the internal counter will trigger a "high" state.

### `bool process(bool current_state, bool previous_state)`
Processes a new sample of the signal.
- `current_state`: The current digital value of the signal.
- `previous_state`: The value of the signal in the previous call.
- **Returns**: `true` if a rising edge was detected (transition from `false` to `true`) and the internal count is less than or equal to `max_threshold`. Otherwise, returns `false`.

## Constraints
- `process` is a real-time method: it must not allocate memory, throw exceptions, use mutexes, or perform I/O.
- The internal counter increments on every rising edge (0 to 1 transition).
