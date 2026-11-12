# DecimationFilter

## Description
The `DecimationFilter` reduces the sample rate of a signal by averaging blocks of $N$ consecutive samples. It implements a simple moving-average decimation (a rectangular window).

## Public API
- `void configure(size_t decimation_factor)`: Initializes the internal buffer to hold the required number of samples. Called once before processing.
- `bool push(float sample, float& output)`: 
    - Pushes a sample into the internal accumulator.
    - If the number of samples collected reaches the `decimation_factor`, it calculates the average, stores it in `output`, resets the accumulator, and returns `true`.
    - Otherwise, it returns `false`.

## Preconditions
- `configure` must be called with `decimation_factor > 0` before calling `push`.

## Real-time Constraints
- The `push` method must not allocate heap memory, throw exceptions, acquire locks, or perform I/O.
