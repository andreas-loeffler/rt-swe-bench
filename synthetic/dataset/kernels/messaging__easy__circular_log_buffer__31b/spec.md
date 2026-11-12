# CircularLogBuffer

The `CircularLogBuffer` provides a fixed-size ring buffer for storing timestamped event strings. It is designed for telemetry logging in real-time systems where memory allocation during the logging phase is prohibited.

## Public API

### `void configure(size_t max_entries, size_t max_string_len)`
Called once to initialize the buffer.
- `max_entries`: The maximum number of log entries the buffer can hold before overwriting the oldest entry.
- `max_string_len`: The maximum number of characters allowed per event string.

### `void push(double timestamp, const char* message)`
Writes a timestamped event into the buffer.
- If the buffer is full, the oldest entry is overwritten (wrap-around).
- If the `message` exceeds `max_string_len`, it is truncated.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `bool read(size_t index, double& timestamp, char* out_message, size_t out_len)`
Reads an entry from the buffer by its current logical index (0 being the oldest, `size()-1` being the newest).
- `out_message` must be provided with `out_len` to store the string.
- Returns `true` if the index is valid and the entry was read; `false` otherwise.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `size_t size()`
Returns the current number of entries stored in the buffer.

## Preconditions
- `configure` must be called before any other method.
- `read` must be called with an `out_message` buffer large enough to hold the stored string to avoid truncation in the output.
