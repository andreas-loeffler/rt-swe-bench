# MedianFilter Spec

The `MedianFilter` removes impulse noise (outliers) from a stream of floating-point samples by maintaining a sliding window of the most recent samples and calculating the median.

## API
- `void configure(size_t window_size)`: Initializes the filter. Allocates memory for a window of the specified size.
- `void push(float sample)`: Adds a new sample to the window. If the window is full, the oldest sample is discarded.
- `float get_median() const`: Returns the median of the current samples in the window.

## Behavior
- The window follows First-In-First-Out (FIFO) logic for samples.
- If the window is empty, `get_median()` returns `0.0f`.
- For a window with $N$ elements:
    - If $N$ is odd, the median is the middle element of the sorted samples.
    - If $N$ is even, the median is the average of the two middle elements of the sorted samples.
- The filter must handle wrap-around of the internal buffer as it fills and overflows.

## Constraints
- `configure` is called once and may allocate.
- `push` and `get_median` are hard real-time methods: they must not allocate heap memory, throw exceptions, use locks, or perform I/O.
