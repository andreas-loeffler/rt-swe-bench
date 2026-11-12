# RingBufferIntegrator

The `RingBufferIntegrator` computes a moving sum (integration) of a sampled input stream over a sliding window of a fixed size. It maintains a history of the last $N$ samples and provides the current integrated sum.

## Public API

### `void configure(size_t window_size)`
- **Purpose**: Initializes the integrator with a window of size `window_size`.
- **Preconditions**: Must be called before any real-time methods.
- **Behavior**: Allocates the necessary memory to store the window.

### `void push(float value)`
- **Purpose**: Adds a new sample to the sliding window.
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.
- **Behavior**: 
    - If the window is not yet full, the sample is added and the sum increases.
    - If the window is full, the oldest sample is replaced by the new sample, and the sum is updated accordingly.

### `float get_sum() const`
- **Purpose**: Returns the current sum of all samples currently stored in the window.
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.
- **Behavior**: Returns the accumulated sum of the samples currently in the buffer (up to `window_size`).

### `void reset()`
- **Purpose**: Clears the buffer and resets the sum to zero.
- **Real-time Constraint**: Must not allocate heap memory, throw, lock, or perform I/O.
- **Behavior**: The buffer is treated as empty; subsequent `push` calls fill the window from the start.
