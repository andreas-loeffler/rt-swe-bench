# SlidingWindowAverage

The `SlidingWindowAverage` class maintains a rolling average of the last `N` samples of floating-point data. It utilizes a circular buffer to ensure that only the most recent samples contribute to the mean.

## Public API

### `void configure(size_t window_size)`
Initializes the internal buffer to hold exactly `window_size` samples. This method is called once during the initialization phase and is permitted to allocate heap memory.

### `void push(double sample)`
Adds a new sample to the sliding window. 
- If the window is not yet full, the sample is added, and the count of active elements increases.
- If the window is full, the oldest sample is evicted and replaced by the new sample.

### `double get_average() const noexcept`
Returns the arithmetic mean of all samples currently residing in the window.
- If no samples have been pushed yet, it returns `0.0`.
- Once the window has `k` samples (where $1 \le k \le N$), it returns the sum of those samples divided by $k$.

## Constraints
- `push` and `get_average` are hard real-time methods.
- They must not allocate heap memory, throw exceptions, use locks, or perform I/O.
- Time and space complexity for `push` and `get_average` must be $O(1)$.
