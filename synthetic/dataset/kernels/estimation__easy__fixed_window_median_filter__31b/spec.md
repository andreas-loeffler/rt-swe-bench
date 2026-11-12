# FixedWindowMedianFilter

The `FixedWindowMedianFilter` implements a sliding window median filter used to remove impulsive noise (spikes) from a sensor data stream. It maintains a window of the last $N$ samples and computes the median of these values for every new sample processed.

## Public API

### `void configure(size_t window_size)`
- Called once during initialization.
- Allocates internal memory required to hold `window_size` samples.
- Precondition: `window_size > 0`.

### `float process(float sample)`
- Real-time method.
- Pushes the provided `sample` into the sliding window, displacing the oldest sample.
- Returns the median of the current window.
- For windows not yet full (during the warm-up phase), the median is computed based on the samples currently available.
- **Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavioral Details
- If the window size $N$ is odd, the median is the middle element of the sorted window.
- If the window size $N$ is even, the median is the average of the two middle elements of the sorted window.
- The filter operates on a First-In-First-Out (FIFO) basis for sample replacement.
