# Envelope Follower

The `EnvelopeFollower` extracts the amplitude envelope of a signal. It implements a rectified signal follower with independent attack and release time constants.

## API
- `void configure(float sampleRate, float attackTimeSec, float releaseTimeSec)`
  Sets the sample rate and the time constants. May allocate memory.
- `float step(float input)`
  Processes a single sample and returns the current envelope level.

## Behavior
1. The input signal is rectified (absolute value).
2. The envelope increases toward the rectified input at the `attack` rate when the input is greater than the current envelope.
3. The envelope decays toward the rectified input at the `release` rate when the input is less than the current envelope.
4. The coefficients are derived as: $\alpha = 1.0 - e^{-1 / (tau \cdot f_s)}$.

## Constraints
- `step` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- `configure` must be called before any call to `step`.
