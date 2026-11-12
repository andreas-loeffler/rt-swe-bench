# LookAheadBuffer

The `LookAheadBuffer` provides a fixed-size ring buffer designed for motion control systems to retrieve waypoints (coordinates) in a real-time loop. It ensures that the motion controller has a stable queue of targets to consume, preventing gaps in motion due to jitter in the waypoint provider.

## Public API

### `void configure(size_t capacity)`
- Called once before real-time execution.
- Allocates an internal buffer of size `capacity`.
- Resets the buffer to an empty state.

### `bool push(double waypoint)`
- **Real-time method.**
- Adds a waypoint to the end of the queue.
- Returns `true` if the waypoint was added successfully.
- Returns `false` if the buffer is full (overflow).
- **Constraint:** Must not allocate, throw, lock, or perform I/O.

### `bool pop(double& out_waypoint)`
- **Real-time method.**
- Retrieves the oldest waypoint from the queue and stores it in `out_waypoint`.
- Returns `true` if a waypoint was successfully retrieved.
- Returns `false` if the buffer is empty (underflow).
- **Constraint:** Must not allocate, throw, lock, or perform I/O.

## Behavior
- The buffer operates as a First-In-First-Out (FIFO) queue.
- Capacity is strict; once `configure` is called, the size cannot be changed without calling `configure` again (which is forbidden on the real-time path).
