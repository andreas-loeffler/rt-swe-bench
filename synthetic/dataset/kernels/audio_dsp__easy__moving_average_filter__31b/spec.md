# MovingAverageFilter

The `MovingAverageFilter` computes the arithmetic mean of the last $N$ samples. It implements a sliding window (FIR filter) where the output for the current sample is the average of the most recent $N$ inputs, including the current one.

## API

### `void configure(size_t window_size)`
Configures the filter with a window size of $N$. This method allocates the necessary memory for the sliding buffer. It must be called before any calls to `process`.

### `float process(float input)`
Adds a new sample to the window and returns the current moving average.
- If the number of samples received since `configure` is $M < N$, the average is calculated over $M$ samples.
- Once $M \ge N$, the average is calculated over the most recent $N$ samples.

## Constraints
- The `process` method runs on a hard real-time path: it must not allocate heap memory, throw exceptions, use locks, or perform I/O.
- Memory is reserved during the `configure` call.
