# PacketBurstAggregator

The `PacketBurstAggregator` collects small incoming data packets into a single large contiguous frame (a "burst") to optimize DMA transfers. It manages a fixed-size internal buffer.

## Public API

- `void configure(size_t max_burst_size)`
  Configures the internal buffer capacity. This is called once before any real-time processing. It may allocate memory.

- `size_t push(const uint8_t* data, size_t len)`
  Appends a packet to the current burst. 
  - If the packet fits entirely within the remaining space of the `max_burst_size`, it is added and `len` is returned.
  - If the packet is larger than the remaining space, it is not added, and `0` is returned (indicating the current burst is effectively "full" and must be popped before more data can be added).
  - **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.

- `size_t pop(uint8_t* out_buffer, size_t out_capacity)`
  Retrieves the aggregated burst.
  - Copies up to `out_capacity` bytes from the internal buffer to `out_buffer`.
  - Resets the internal buffer pointer to the start (clearing the burst).
  - Returns the number of bytes copied.
  - **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.

## Preconditions
- `configure` must be called before `push` or `pop`.
- `out_buffer` passed to `pop` must be pre-allocated and large enough to hold the expected data.
