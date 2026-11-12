# SineWaveOscillator

Generates a sine wave based on a phase accumulator.

## Public API

### `void configure(double sampleRate, double frequency)`
Configures the oscillator with a sample rate in Hz and a target frequency in Hz. This method may allocate memory or perform heavy calculations.

### `double process()`
Advances the oscillator by one sample and returns the current amplitude at that point.

## Preconditions
- `sampleRate` must be greater than 0.
- `frequency` must be non-negative.

## Real-Time Constraints
The `process()` method must run on a hard real-time path. It must not allocate heap memory, throw exceptions, use mutexes, or perform I/O operations.
