# SignalPeakAccumulator

## Description
The `SignalPeakAccumulator` collects a batch of floating-point samples and tracks the maximum absolute magnitude encountered. Once the internal buffer reaches its configured capacity, the accumulator returns the peak magnitude of that batch and resets its state to begin collecting the next batch.

## API
- `void configure(size_t batch_size)`
  Configures the number of samples to collect before reporting the peak. This method is called once during initialization and may allocate memory.
- `float process(float sample)`
  Processes a single sample. 
  - If the internal count is less than `batch_size - 1`, it returns `0.0f` and updates the internal peak.
  - If the internal count reaches `batch_size - 1`, it updates the peak with the current sample, returns the final peak magnitude for the batch, and resets the accumulator for the next batch.

## Constraints
- `process` is a hard real-time method: it must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Preconditions
- `configure` must be called before any call to `process`.
- `batch_size` must be greater than 0.
