# TrajectorySmoother

The `TrajectorySmoother` provides a real-time low-pass filter for motion trajectory setpoints using a moving average (box filter) over a sliding window of the most recent inputs.

## API

### `void configure(size_t window_size)`
Initializes the internal ring buffer to hold `window_size` samples. This is the only method allowed to allocate memory.

### `void push(float setpoint)`
Adds a new trajectory setpoint to the buffer. If the buffer is full, the oldest sample is replaced. This method is hard real-time.

### `float get_smoothed()` const
Returns the arithmetic mean of all available samples in the buffer. If the buffer is empty, it returns 0.0f. This method is hard real-time.

## Requirements
- `push` and `get_smoothed` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- The smoother must handle the "warm-up" period (where fewer than `window_size` samples have been pushed) by averaging only the available samples.
- The implementation must correctly handle the ring buffer wrap-around.
