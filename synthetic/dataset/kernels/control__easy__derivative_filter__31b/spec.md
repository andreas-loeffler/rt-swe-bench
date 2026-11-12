# DerivativeFilter

The `DerivativeFilter` computes the rate of change (derivative) of an incoming signal over a specified time interval, combined with a first-order low-pass filter to suppress high-frequency noise.

## Public API

### `void configure(double sample_rate, double cutoff_freq)`
Prepares the filter for processing.
- `sample_rate`: The sampling frequency of the input signal in Hz.
- `cutoff_freq`: The -3dB cutoff frequency of the low-pass filter in Hz.

### `double process(double input)`
Processes a single sample.
- **Input**: A single signal sample.
- **Output**: The filtered derivative of the signal.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
The kernel computes the instantaneous derivative: 
$raw\_derivative = (input_{n} - input_{n-1}) \times sample\_rate$

It then applies a first-order IIR low-pass filter:
$output_{n} = \alpha \times raw\_derivative + (1 - \alpha) \times output_{n-1}$
where $\alpha = \frac{2\pi \cdot cutoff\_freq}{sample\_rate + 2\pi \cdot cutoff\_freq}$.
