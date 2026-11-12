# HysteresisDebouncerArray

The `HysteresisDebouncerArray` processes a batch of digital input channels. Each channel must maintain a stable state (true or false). A state change is only registered if the input remains at the new value for a continuous duration exceeding a configured time threshold. If the input fluctuates before the threshold is reached, the timer resets.

## Public API

### `void configure(size_t num_channels, double sample_period_sec, double debounce_time_sec)`
- `num_channels`: Number of digital input signals to track.
- `sample_period_sec`: The time interval between calls to `process`.
- `debounce_time_sec`: The required duration of stability to trigger a state change.
- Precondition: Called once before any real-time calls. May allocate memory.

### `void process(const bool* inputs, bool* outputs)`
- `inputs`: Array of current raw digital readings (size `num_channels`).
- `outputs`: Array where the debounced state for each channel is written (size `num_channels`).
- Precondition: `configure` must have been called. Both arrays must be at least `num_channels` in size.
- Real-time Constraint: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
For each channel $i$:
1. If `inputs[i]` is equal to the current `stabilized_state[i]`, the timer for channel $i$ is reset to 0.
2. If `inputs[i]` is different from `stabilized_state[i]`, the timer increments by `sample_period_sec`.
3. If the timer reaches or exceeds `debounce_time_sec`, the `stabilized_state[i]` is flipped to match `inputs[i]` and the timer is reset.
