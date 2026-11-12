# CircularLog

A high-frequency telemetry event buffer that stores fixed-size telemetry records in a wrap-around circular buffer.

## API

### `void configure(size_t capacity)`
Pre-allocates the internal buffer to hold `capacity` elements.

### `bool push(uint32_t id, float value)`
Inserts a new telemetry event. If the buffer is full, the oldest entry is overwritten. Returns `true` if the push succeeded (always true if capacity > 0).

### `bool pop(uint32_t& out_id, float& out_value)`
Removes and returns the oldest telemetry event. Returns `false` if the buffer is empty.

### `size_t size()`
Returns the number of elements currently in the buffer (0 to `capacity`).

## Constraints
- All methods except `configure` are real-time and must not allocate, lock, throw, or perform I/O.
- The buffer is thread-unsafe (single-threaded real-time context).
- If `capacity` is 0, `push` returns `false`.
