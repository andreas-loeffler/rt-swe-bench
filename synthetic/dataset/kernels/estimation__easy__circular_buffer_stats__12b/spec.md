# CircularBufferStats

Computes the **median** of a continuous stream of real-time data stored in a fixed-size circular buffer.

## API

### `void configure(size_t capacity)`
Sets the capacity of the circular buffer. 
- **Preconditions**: `capacity > 0`.
- **Side Effects**: Allocates memory for the buffer.

### `double process(double value)`
Pushes a new value into the circular buffer (overwriting the oldest if full) and returns the current median of all values currently stored in the buffer.
- **Real-time constraints**: Must not allocate, lock, or perform I/O.
- **Behavior**: 
    - If the buffer is empty, the median of the first element is that element.
    - As elements are added, the median is calculated based on the elements currently residing in the buffer (up to `capacity`).
- **Returns**: The median as a `double`.
