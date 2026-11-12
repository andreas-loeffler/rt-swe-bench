# SlidingWindowChecksum

Computes a rolling checksum of a byte stream using a sliding window of fixed size. The checksum is defined as the sum of all bytes currently within the window, treating bytes as unsigned 8-bit integers and the accumulator as a 64-bit unsigned integer to prevent overflow during the window's lifetime.

## Public API

- `void configure(size_t window_size)`
  Sets the size of the sliding window. This is the only method allowed to allocate memory.
- `void push(uint8_t byte)`
  Adds a new byte to the end of the stream. If the number of bytes pushed exceeds the window size, the oldest byte is removed from the checksum calculation. This method must not allocate, throw, lock, or perform I/O.
- `uint64_t current_checksum()`
  Returns the current sum of the bytes within the sliding window. This method must not allocate, throw, lock, or perform I/O.
- `size_t current_window_fill()`
  Returns the number of bytes currently contributing to the checksum (between 0 and `window_size`). This method must not allocate, throw, lock, or perform I/O.

## Preconditions
- `configure` must be called exactly once before any calls to `push`, `current_checksum`, or `current_window_fill`.
- `window_size` must be greater than 0.
