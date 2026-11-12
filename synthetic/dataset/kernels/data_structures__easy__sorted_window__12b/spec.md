# SortedWindow

Maintains a fixed-size sorted collection of the most recent values. This is used for calculating rolling medians or other order statistics in real-time.

## API

### `void configure(size_t capacity)`
Initializes the internal buffer to a fixed size.
- **Preconditions**: `capacity > 0`.
- **Note**: May allocate memory.

### `double process(double value)`
Adds a new value to the window (evicting the oldest if full) and returns the current median of the window.
- **Preconditions**: `configure` must be called with `capacity > 0`.
- **Real-time**: This method must not allocate, lock, or perform I/O.

## Behavior
1. If the window is not yet full, the new value is inserted into its sorted position.
2. If the window is full, the oldest value (the one added first) is removed, and the new value is inserted into the sorted position.
3. The median is calculated as:
   - The middle element for odd-sized windows.
   - The average of the two middle elements for even-sized windows.
4. The implementation must maintain the sorted order of elements in the buffer at all times.
