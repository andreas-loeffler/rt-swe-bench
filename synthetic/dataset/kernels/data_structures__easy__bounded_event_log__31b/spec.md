# BoundedEventLog

The `BoundedEventLog` provides a lock-free, allocation-free circular buffer for logging fixed-size event structures. It is designed for use in real-time contexts where events must be recorded without triggering non-deterministic delays.

## Public API

### `template <typename EventT> class BoundedEventLog`

#### `void configure(size_t capacity)`
Called once before any real-time calls. Allocates the internal buffer to hold exactly `capacity` elements of type `EventT`.

#### `void push(const EventT& event)`
**Real-time method.** Logs an event into the circular buffer. If the buffer is full, the oldest event is overwritten (wrap-around). 
- **Precondition:** `configure()` must have been called.
- **Constraint:** Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

#### `size_t size() const`
**Real-time method.** Returns the number of events currently stored in the log (up to the capacity).
- **Constraint:** Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

#### `bool pop(EventT& out_event)`
**Real-time method.** Retrieves and removes the oldest event from the log. Returns `true` if an event was retrieved, `false` if the log is empty.
- **Precondition:** `configure()` must have been called.
- **Constraint:** Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
