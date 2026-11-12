# MedianFilter

## Overview
The `MedianFilter` implements a sliding window median filter used for removing impulse noise (salt-and-pepper noise) from a signal. It maintains a circular buffer of the last $N$ samples and computes the median of the current window.

## API
- `void configure(size_t window_size)`: Initializes the filter window size and allocates necessary internal buffers. Must be called once before any real-time calls.
- `void push(float sample)`: Adds a new sample to the window. If the window is full, the oldest sample is discarded.
- `float process()`: Returns the median of the current window. If the window is empty, it returns `0.0f`.

## Constraints and Behavior
- **Real-time Constraint**: The `push` and `process` methods must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- **Windowing**: The filter uses a first-in, first-out (FIFO) mechanism.
- **Median Calculation**: 
    - For an odd number of elements $n$, the median is the middle element of the sorted window.
    - For an even number of elements $n$, the median is the average of the two middle elements.
- **Initialization**: Before the window is fully populated (i.e., fewer than `window_size` samples have been pushed), the median is calculated based on the number of samples currently available.
