# SlidingWindowAverage

Computes the arithmetic mean of the last N samples stored in a circular buffer.

## Public API
- `void configure(size_t window_size)`: Initializes the internal buffer to hold exactly `window_size` samples.
- `double push(double sample)`: Adds a new sample to the window and returns the current average of all samples currently in the buffer.

## Behavior
- If the buffer is not yet full, the average is calculated based on the number of samples actually pushed so far (1 to N).
- Once the buffer reaches `window_size`, the oldest sample is evicted to make room for the new one, and the average is calculated over the last `window_size` samples.
- `configure` must be called before any calls to `push`.

## Real-time Constraints
- `push` is a hard real-time method. It must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
