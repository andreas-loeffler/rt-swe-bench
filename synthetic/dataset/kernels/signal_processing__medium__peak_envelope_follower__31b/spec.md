# PeakEnvelopeFollower

The `PeakEnvelopeFollower` tracks the amplitude envelope of a signal. It implements a non-linear filter that tracks the absolute value of the input signal, rising quickly (attack) and falling slowly (release).

## Public API

### `void configure(float attack_time_s, float release_time_s, float sample_rate_hz)`
Configures the filter coefficients based on the time constants and the sampling rate. This method is called once before processing and is allowed to allocate memory.

### `float process(float input)`
Processes a single sample of the input signal and returns the current envelope level.
- **Behavior**: 
    - Takes the absolute value of the input.
    - If the absolute input is greater than the current envelope, it moves toward the input using the attack coefficient.
    - If the absolute input is less than the current envelope, it moves toward the input using the release coefficient.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `float get_level() const`
Returns the current value of the envelope without updating it.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `void reset()`
Resets the internal envelope state to 0.0.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Mathematical Model
The envelope $E_n$ at sample $n$ is updated as:
1. Let $x = |input_n|$
2. If $x > E_{n-1}$: $E_n = E_{n-1} + \text{attack\_coeff} \cdot (x - E_{n-1})$
3. If $x \le E_{n-1}$: $E_n = E_{n-1} + \text{release\_coeff} \cdot (x - E_{n-1})$

Coefficients are derived from: $\text{coeff} = 1.0 - \exp(-1.0 / (\text{time} \cdot \text{sample\_rate}))$
