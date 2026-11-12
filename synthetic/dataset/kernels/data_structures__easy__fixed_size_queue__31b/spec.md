# FixedSizeQueue

A fixed-size, first-in-first-out (FIFO) queue for a single-threaded real-time context.

## Public API
- `void configure(size_t capacity)`: Initializes the queue with a fixed capacity. May allocate memory.
- `bool push(T value)`: Adds an element to the end of the queue. Returns `true` if successful, or `false` if the queue is full.
- `bool pop(T& value)`: Removes the element from the front of the queue and stores it in `value`. Returns `true` if successful, or `false` if the queue is empty.

## Constraints
- `push` and `pop` are hard real-time methods. They must not allocate heap memory, throw exceptions, use locks, or perform I/O.
- The queue behaves as a circular buffer.

## Preconditions
- `configure` must be called exactly once before any calls to `push` or `pop`.
