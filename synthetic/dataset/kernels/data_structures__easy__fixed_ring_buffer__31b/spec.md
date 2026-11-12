# FixedRingBuffer

The `FixedRingBuffer` is a first-in, first-out (FIFO) data structure with a fixed capacity. It is designed for real-time systems where memory allocation is forbidden during the processing cycle.

## API

### `void configure(size_t capacity)`
Initializes the buffer to hold up to `capacity` elements. This method is called once and is allowed to allocate memory.

### `T push(T value)`
Pushes a new value to the end of the buffer. 
- If the buffer is not yet full, it returns a default-constructed value of type `T`.
- If the buffer is full, it returns the oldest value (the one being overwritten) and then replaces it with the new value.

## Constraints
- `push` must be hard real-time: no heap allocation, no throwing, no locking, and no I/O.
- The behavior is undefined if `push` is called before `configure`.
