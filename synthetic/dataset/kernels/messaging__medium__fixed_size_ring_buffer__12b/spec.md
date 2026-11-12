# FixedRingBuffer

A thread-safe (single-producer, single-consumer) fixed-size circular buffer for sensor data.

## API

### `void configure(size_t capacity)`
Configures the buffer with a fixed capacity. This is the only method allowed to allocate memory.

### `bool push(const T& item)`
Pushes an item onto the buffer. 
- Returns `true` if successful.
- Returns `false` if the buffer is full.
- **Real-time:** Must not allocate, lock, or perform I/O.

### `bool pop(T& item)`
Pops an item from the buffer into the provided reference.
- Returns `true` if an item was retrieved.
- Returns `false` if the buffer is empty.
- **Real-time:** Must not allocate, lock, or perform I/O.

## Constraints
- The buffer is a Ring Buffer.
- The implementation must be thread-safe for Single-Producer (push) and Single-Consumer (pop) concurrency.
- Real-time methods (`push`, `pop`) must be O(1).
- Real-time methods must not allocate heap memory, throw exceptions, use mutexes, or perform I/O.
