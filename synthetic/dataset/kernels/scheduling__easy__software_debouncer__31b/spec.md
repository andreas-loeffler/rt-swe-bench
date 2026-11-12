# SoftwareDebouncer

The `SoftwareDebouncer` filters a noisy binary input signal. It prevents rapid state transitions (chatter) by requiring the input signal to remain stable (constant) for a specified number of consecutive cycles before the output state is updated to match the input.

## Public API

### `void configure(uint32_t required_cycles)`
- **Purpose**: Initializes the debouncer with the stability threshold.
- **Preconditions**: Must be called exactly once before any calls to `process`.
- **Behavior**: Sets the number of cycles the input must be stable to trigger a state change.

### `bool process(bool input)`
- **Purpose**: Processes a single sample of the binary input.
- **Preconditions**: `configure` must have been called.
- **Behavior**: 
    - If the `input` matches the current internal stable state, the counter is reset to 0 and the current state is returned.
    - If the `input` differs from the current internal stable state, a counter is incremented.
    - If the counter reaches `required_cycles`, the internal stable state is updated to the new input value and the counter is reset.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Logic Summary
- Output changes only after `required_cycles` of consistent, opposite input.
- Any deviation from the "target" state during the counting period resets the counter to 0.
