# SampleAccumulator

A real-time kernel that sums a series of incoming signal values over a fixed-size sliding window.

## API

### `void configure(size_t window_size)`
Sets the number of samples to include in the sum. 
- **Preconditions**: `window_size > 0`.
- **Behavior**: Allocates a buffer to hold the window. If `window_size` is 0, the internal buffer will be empty.

### `double process(double sample)`
Processes a single sample by adding it to the running sum, discarding the oldest sample if the window is full.
- **Returns**: The current sum of all samples in the window.
- **Real-time Constraint**: Must not allocate memory, lock, or perform I/O.

## Behavior
1. If the number of samples received is less than `window_size`, the result is the sum of all samples received so far.
2. Once the number of samples exceeds `window_size`, the kernel maintains a sliding window. When a new sample arrives, the oldest sample is subtracted from the sum and the new sample is added.
3. The kernel handles the "warm-up" phase automatically.
