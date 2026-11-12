# WeightedMovingAverage

The `WeightedMovingAverage` class implements a signal smoothing filter that calculates a weighted average of the last $N$ samples. It supports two weighting modes: Linear and Gaussian.

## Public API

### Configuration
- `void configure(size_t window_size, WeightMode mode, float sigma = 1.0f)`
  - `window_size`: The number of samples to consider for the average.
  - `mode`: Either `WeightMode::Linear` or `WeightMode::Gaussian`.
  - `sigma`: Used only for Gaussian weights to control the bell curve width.
  - **Precondition**: Must be called before any real-time methods.

### Real-Time Methods
- `void push(float sample)`
  - Adds a new sample to the circular buffer.
  - **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.
- `float process()`
  - Computes the weighted average of the current window.
  - If the buffer is not yet full, it computes the average of available samples, normalizing weights accordingly.
  - **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.
- `void reset()`
  - Clears the buffer and resets the internal state.
  - **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.

## Behavior
- **Linear Mode**: Weights decrease linearly from the most recent sample (weight = $W$) to the oldest (weight = $W - (N-1)$).
- **Gaussian Mode**: Weights follow a Gaussian distribution centered at the most recent sample.
- **Normalization**: The filter always divides the weighted sum by the sum of weights actually used (handling the warm-up period where the buffer is partially full).
