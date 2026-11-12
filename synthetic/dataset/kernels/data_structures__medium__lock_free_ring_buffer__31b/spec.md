# LockFreeRingBuffer

A thread-safe, single-producer single-consumer (SPSC) lock-free ring buffer designed for high-performance data transfer between a producer thread and a consumer thread in real-time systems.

## Public API

### `void configure(size_t capacity)`
Initializes the internal buffer to hold up to `capacity` elements. This method is called once before any real-time operations and is permitted to allocate memory.

### `bool push(T value)`
Attempts to add an element to the buffer.
- **Returns**: `true` if the value was successfully added; `false` if the buffer is full.
- **Preconditions**: `configure` must have been called.
- **Real-time Constraint**: Must not allocate, lock, throw, or perform I/O.

### `bool pop(T& result)`
Attempts to remove an element from the buffer and store it in `result`.
- **Returns**: `true` if a value was successfully retrieved; `false` if the buffer is empty.
- **Preconditions**: `configure` must have called.
- **Real-time Constraint**: Must not allocate, lock, throw, or perform I/O.

## Behavior
- The buffer operates on a First-In-First-Out (FIFO) basis.
- Wrap-around logic is handled internally to reuse the fixed-size allocation.
- The buffer is "full" when adding one more element would cause the write index to overlap with the read index.
