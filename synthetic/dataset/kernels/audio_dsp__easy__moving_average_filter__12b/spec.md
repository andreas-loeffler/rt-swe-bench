# MovingAverageFilter

Computes a running average of the last $N$ samples from a stream of floating-point values. This filter is used to smooth out high-frequency noise in sensor data or audio signals.

## API

### `void configure(size_t window_size)`
Initializes the internal buffer. 
- **Preconditions**: `window_size` must be greater than 0.
- **Note**: This is the only method allowed to allocate memory.

### `float process(float sample)`
Processes a single input sample and returns the current average.
- If the number of samples processed is less than `window_size`, the average is calculated based on the samples received so far.
- **Real-time Constraint**: This method must not allocate memory, lock, or perform I/O.

## Behavior
- The filter uses a FIFO buffer to maintain the last $N$ samples.
- For each call to `process(sample)`, the oldest sample is replaced by the new sample.
- The return value is the arithmetic mean of all samples currently in the buffer.
- If `window_size` is 1, the output is equal to the input.
