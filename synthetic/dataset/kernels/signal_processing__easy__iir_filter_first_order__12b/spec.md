# IirFilter (First-Order)

Computes a first-order Infinite Impulse Response (IIR) filter. This is commonly used for basic low-pass or high-pass filtering of signal data.

## API

### `void configure(float alpha)`
Configures the filter coefficients.
- `alpha`: The smoothing factor (typically $0 < \alpha < 1$).

### `float process(float input)`
Processes a single sample through the filter.
- `input`: The raw input sample.
- Returns: The filtered output sample.

## Preconditions
- `configure` must be called before any calls to `process`.
- `alpha` should be in the range $(0, 1)$ for stable behavior.

## Real-time Constraints
- `process` is a real-time method: it must not allocate, throw, lock, or perform I/O.
