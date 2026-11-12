# RingBufferLogger

The `RingBufferLogger` provides a mechanism to store a fixed-size window of the most recent signal samples for deferred analysis. It operates as a circular buffer, overwriting the oldest data once the buffer is full.

## Public API

### `void configure(size_t capacity)`
Allocates internal resources to store up to `capacity` samples. This method is called once during initialization.

### `void push(float sample)`
Adds a new sample to the logger. If the buffer is full, the oldest sample is overwritten. This method is hard real-time: it must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `size_t size() const`
Returns the number of samples currently stored in the buffer (up to the configured capacity). This method is hard real-time.

### `float get(size_t index) const`
Returns the sample at the specified index, where `index = 0` is the oldest sample currently stored and `index = size() - 1` is the most recent.
- **Precondition**: `index < size()`.
- This method is hard real-time.

## Behavioral Requirements
- The buffer must handle wrap-around internally.
- The `size()` must increase from 0 up to `capacity` and then remain constant.
- Access via `get()` must provide samples in chronological order (oldest to newest).
