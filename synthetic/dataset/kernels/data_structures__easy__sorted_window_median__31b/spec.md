# SortedWindowMedian

Maintains a rolling window of the last N samples and provides the median value of that window.

## API
- `void configure(size_t window_size)`: Initializes the internal buffer to hold `window_size` samples.
- `double push(double sample)`: Adds a new sample to the window, discarding the oldest sample if the window is full, and returns the median of the current window.

## Behavior
- The window acts as a First-In-First-Out (FIFO) queue.
- The median is calculated as the middle element of the sorted samples currently in the window.
- If the window size is even, the median is the average of the two middle elements.
- If the window is not yet full, the median is calculated based on the number of elements currently present.
- If the window size is configured to 0, `push` returns 0.0.

## Real-time Constraints
- `push` must not allocate heap memory, throw exceptions, acquire locks, or perform I/O.
