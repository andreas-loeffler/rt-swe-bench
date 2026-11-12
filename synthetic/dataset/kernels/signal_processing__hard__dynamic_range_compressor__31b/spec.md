# DynamicRangeCompressor

The `DynamicRangeCompressor` reduces the volume of signals that exceed a specified amplitude threshold. It implements a feed-forward compressor with attack and release smoothing to prevent abrupt gain changes.

## Public API

### `void configure(float threshold_db, float ratio, float attack_ms, float release_ms, float sample_rate)`
- `threshold_db`: The level (in dBFS, usually negative) above which compression begins.
- `ratio`: The compression ratio (e.g., 4.0 for 4:1).
- `attack_ms`: Time constant for gain reduction to take effect.
- `release_ms`: Time constant for gain to return to unity.
- `sample_rate`: The sampling rate of the audio signal.

### `float process(float input)`
Processes a single sample.
- **Input**: A single floating-point sample.
- **Output**: The compressed sample.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
1. **Level Detection**: The absolute value of the input is used to determine the current signal level.
2. **Gain Calculation**: 
   - If the signal is below the threshold, the target gain is $1.0$.
   - If the signal is above the threshold, the gain is reduced such that the output level is compressed by the given ratio relative to the threshold.
3. **Smoothing**: The target gain is applied to a gain-smoothing filter (an exponential moving average) using the attack time constant when gain is decreasing and the release time constant when gain is increasing.
4. **Application**: The smoothed gain is multiplied by the original input.

## Preconditions
- `configure` must be called before any call to `process`.
- `ratio` must be $\ge 1.0$.
- `sample_rate`, `attack_ms`, and `release_ms` must be positive.
