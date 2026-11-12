# MovingAverageFilter

The `MovingAverageFilter` computes the arithmetic mean of the most recent $N$ samples provided to it.

## Public API

- `void configure(size_t window_size)`
  Configures the filter to use a window of $N$ samples. This method is called once before any real-time processing and may allocate memory.
- `double process(double sample)`
  Adds a new sample to the filter and returns the mean of the last $N$ samples. If fewer than $N$ samples have been processed, it returns the mean of all samples seen so far.

## Constraints

- `process` is a hard real-time method: it must not allocate heap memory, throw exceptions, use locks, or perform I/O.
- Memory is reserved during the `configure` call.

## Behavior

- The filter uses a sliding window of size $N$.
- When the buffer is not yet full, the output is $\frac{1}{k} \sum_{i=1}^{k} x_i$, where $k$ is the number of samples processed.
- When the buffer is full, the output is $\frac{1}{N} \sum_{i=k-N+1}^{k} x_i$.
