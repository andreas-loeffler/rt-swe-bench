# FixedStringBuilder

The `FixedStringBuilder` class constructs formatted status strings into a pre-allocated, fixed-size internal buffer. It is designed for real-time systems where dynamic memory allocation during the control loop is prohibited.

### Public API

#### `void configure(size_t buffer_capacity)`
Initializes the internal buffer to the specified capacity. 
- **Preconditions**: `buffer_capacity > 0`.
- **Behavior**: Allocates the internal buffer. This is the only method allowed to allocate memory.

#### `size_t append(std::string_view text)`
Appends a string to the internal buffer.
- **Real-time constraints**: Must not allocate, throw, lock, or perform I/O.
- **Behavior**: 
    - If the text fits in the remaining space, it is copied into the buffer.
    - If the text exceeds the remaining space (including the null terminator), the function returns the amount of text that *could* be appended, but only the portion that fits is written.
    - If the buffer is already full (no room for even the first char), it returns 0.
    - The buffer is always null-terminated.

#### `std::string_view view()`
Returns a view of the current content of the buffer.
- **Real-time constraints**: Must not allocate, throw, lock, or perform I/O.
- **Behavior**: Returns a `std::string_view` starting from the beginning of the buffer up to (but not including) the null terminator.

#### `void clear()`
Resets the buffer state.
- **Real-time constraints**: Must not allocate, throw, lock, or perform I/O.
- **Behavior**: Resets the internal write pointer to 0. The old content remains in the buffer but is ignored by subsequent `append` calls.
