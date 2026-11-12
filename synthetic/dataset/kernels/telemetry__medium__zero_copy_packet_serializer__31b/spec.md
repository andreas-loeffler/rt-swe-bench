# ZeroCopyPacketSerializer

The `ZeroCopyPacketSerializer` is designed for high-performance telemetry systems where data must be packed into a raw byte buffer for transmission without heap allocation or redundant copying during the real-time loop.

## Public API

### `void configure(size_t max_buffer_size)`
Called once during initialization. Allocates internal state necessary to manage a buffer of the specified maximum size.

### `bool serialize_float(float value)`
Appends a 32-bit float to the internal buffer using the system's native endianness. 
- Returns `true` if the value was successfully written.
- Returns `false` if adding the value would exceed the `max_buffer_size` configured.

### `bool serialize_uint32(uint32_t value)`
Appends a 32-bit unsigned integer to the internal buffer.
- Returns `true` if the value was successfully written.
- Returns `false` if adding the value would exceed the `max_buffer_size` configured.

### `size_t get_written_bytes() const`
Returns the current offset (number of bytes written) in the buffer.

### `void reset()`
Resets the write offset to zero, effectively clearing the buffer for a new packet.

### `const uint8_t* data() const`
Returns a pointer to the start of the serialized byte buffer.

## Constraints and Preconditions
- `configure` must be called before any other method.
- The real-time methods (`serialize_float`, `serialize_uint32`, `get_written_bytes`, `reset`, `data`) must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- Once the buffer is full (indicated by `serialize` methods returning `false`), no further data can be written until `reset()` is called.
