# FixedCapacityCommandQueue

The `FixedCapacityCommandQueue` manages a fixed-capacity, lock-free Single-Producer Single-Consumer (SPSC) FIFO queue of command packets. It is designed for domain messaging between a non-real-time thread (producer) and a real-time thread (consumer), or between two real-time threads.

## API

### `void configure(size_t capacity)`
- Called once before real-time execution.
- Reserves memory for the queue to hold up to `capacity` packets.
- Precondition: Must be called before `push` or `pop`.

### `bool push(uint32_t id, float value)`
- Adds a command packet (ID and value) to the tail of the queue.
- Returns `true` if the packet was successfully queued.
- Returns `false` if the queue is full (saturation handling).
- **Real-time constraint**: Must not allocate, lock, throw, or perform I/O.

### `bool pop(uint32_t& id, float& value)`
- Removes the command packet from the head of the queue and populates the output parameters.
- Returns `true` if a packet was retrieved.
- Returns `false` if the queue is empty.
- **Real-time constraint**: Must not allocate, lock, throw, or perform I/O.

## Behavior
- The queue operates as a FIFO.
- If the queue is full, `push` fails immediately without blocking or overwriting existing data.
- Memory is managed as a ring buffer to handle wrap-around logic.
