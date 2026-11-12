# Exponential Moving Average Bank (EMABank)

The `EMABank` class implements a set of first-order Infinite Impulse Response (IIR) filters, one for each channel in a multi-channel stream. It computes the Exponential Moving Average (EMA) to smooth input signals.

## Mathematical Operation
For each channel $i$:
$y[n] = \alpha_i \cdot x[n] + (1 - \alpha_i) \cdot y[n-1]$
where $\alpha_i$ is the smoothing coefficient for channel $i$ ($0 \le \alpha_i \le 1$).

## Public API

### `void configure(size_t num_channels, const float* coefficients)`
- **Purpose**: Initializes the filter bank for a specific number of channels and assigns smoothing coefficients.
- **Preconditions**: `coefficients` must point to an array of at least `num_channels` floats.
- **Allocation**: May allocate heap memory to store state and coefficients.

### `void process(const float* inputs, float* outputs, size_t num_samples)`
- **Purpose**: Processes a block of samples for all channels.
- **Input**: `inputs` is a pointer to an array of size `num_channels * num_samples` (interleaved: channel 0 sample 0, channel 1 sample 0, ..., channel 0 sample 1, ...).
- **Output**: `outputs` is populated with the filtered values in the same interleaved format.
- **Constraints**: `num_samples` must be greater than 0.
- **Real-time**: Must not allocate, lock, throw, or perform I/O.

### `void reset()`
- **Purpose**: Resets the internal state ($y[n-1]$) of all channels to zero.
- **Real-time**: Must not allocate, lock, throw, or perform I/O.

### `float get_state(size_t channel)`
- **Purpose**: Returns the current internal state of a specific channel.
- **Real-time**: Must not allocate, lock, throw, or perform I/O.
