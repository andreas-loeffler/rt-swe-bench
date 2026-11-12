# WindowedBuffer

The `WindowedBuffer` class implements a sample accumulator for audio DSP. It collects incoming samples into a fixed-size buffer. Once the buffer is full, it applies a Hanning window function to the accumulated samples and makes the windowed frame available for processing.

## API

### `void configure(size_t frame_size)`
- Called once before real-time processing.
- Allocates memory for a buffer of `frame_size` samples and pre-calculates the Hanning window coefficients.
- **Precondition**: `frame_size` must be greater than 0.

### `bool push(float sample)`
- Pushes a single sample into the accumulator.
- Returns `true` if the buffer has just become full (a new frame is ready), and `false` otherwise.
- **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.

### `const float* get_frame()`
- Returns a pointer to the buffer containing the windowed samples of the most recent full frame.
- **Real-time constraint**: Must not allocate, throw, lock, or perform I/O.

## Behavior
1. Samples are collected sequentially.
2. When the `push` count reaches `frame_size`, the buffer is "locked" for reading.
3. The windowing operation is applied: `output[n] = input[n] * (0.5 * (1 - cos(2 * PI * n / (frame_size - 1))))`.
4. The internal index resets to 0 for the next frame.
5. `get_frame()` provides access to the most recently processed windowed block.
