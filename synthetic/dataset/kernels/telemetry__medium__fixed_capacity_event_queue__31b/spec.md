# FixedCapacityEventQueue

The `FixedCapacityEventQueue` is a lock-free, single-producer single-consumer (SPSC) ring buffer designed for domain telemetry. It allows a real-time thread to enqueue telemetry events and a non-real-time thread to dequeue them for logging or transmission without causing heap allocations or blocking the real-time path.

## Public API

### `void configure(size_t capacity)`
- **Purpose**: Initializes the internal buffer to hold up to `capacity` events.
- **Preconditions**: Must be called before any real-time method.
- **Behavior**: Allocates memory for the ring buffer.

### `bool push(const TelemetryEvent& event)`
- **Purpose**: Adds a telemetry event to the queue.
- **Preconditions**: `configure` must have been called.
- **Returns**: `true` if the event was successfully enqueued; `false` if the queue is full (saturation).
- **Real-time constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `bool pop(TelemetryEvent& event)`
- **Purpose**: Removes and returns the oldest telemetry event from the queue.
- **Preconditions**: `configure` must have been called.
- **Returns**: `true` if an event was successfully dequeued; `false` if the queue is empty.
- **Real-time constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
- The queue operates as a First-In-First-Out (FIFO) buffer.
- When the queue is full, `push` returns `false` and the event is discarded.
- When the queue is empty, `pop` returns `false`.
- Wrap-around is handled internally via the ring buffer logic.
