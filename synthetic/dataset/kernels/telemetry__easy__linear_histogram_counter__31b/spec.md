# LinearHistogramCounter

The `LinearHistogramCounter` computes a frequency distribution of integer values falling within a specified range.

## API

### `void configure(int32_t min_value, int32_t max_value)`
Initializes the counter. 
- `min_value`: The inclusive lower bound of the histogram.
- `max_value`: The inclusive upper bound of the histogram.
- Precondition: `max_value >= min_value`.
- This method may allocate memory to create the bins.

### `void increment(int32_t value)`
Increments the count for the bin corresponding to the provided value.
- If `value` is within `[min_value, max_value]`, the corresponding bin is incremented.
- If `value` is outside this range, the increment is ignored.
- **Real-time requirement**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `uint64_t get_count(int32_t value) const`
Returns the current count for the bin corresponding to the provided value.
- If `value` is outside the configured range, returns 0.
- **Real-time requirement**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `void reset()`
Sets all bin counters back to zero.
- **Real-time requirement**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
