# TimeoutMonitor

A real-time kernel that monitors the timing of a sequence of events using a sliding window to detect if the average inter-arrival time exceeds a specified threshold.

## API

### `void configure(size_t window_size, double threshold_ms)`
Initializes the monitor. 
- `window_size`: The number of recent events to include in the average calculation.
- `threshold_ms`: The maximum allowable average inter-arrival time in milliseconds.
This method may allocate memory.

### `bool push(double timestamp_ms)`
Records a new event timestamp (in milliseconds). Returns `true` if the event was successfully recorded, `false` otherwise.
**Real-time constraint:** Must not allocate, lock, or perform I/O.

### `bool check_timeout()`
Evaluates the sliding window of timestamps. 
- Returns `true` if the average inter-arrival time of the events currently in the window exceeds `threshold_ms`.
- Returns `false` otherwise.
- If the window contains fewer than 2 events, it returns `false`.
**Real-time constraint:** Must not allocate, lock, or perform I/O.

**Preconditions:**
- `window_size` must be greater than 1.
- `timestamp_ms` should be monotonically increasing.
