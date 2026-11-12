# LinearGainRamp

Computes a linear gain ramp over a buffer of audio samples. Given a starting gain, an ending gain, and a block size, the kernel calculates the step size per sample such that the gain transitions linearly from start to end across the provided buffer.

## API

### `void configure(size_t max_block_size)`
Pre-allocates the internal buffer. 
- `max_block_size`: The maximum number of samples to process in a single `process` call.

### `void process(const float* input, float* output, size_t n, float start_gain, float end_gain)`
Processes `n` samples. For each sample $i \in [0, n)$, the output is $input[i] \times \text{gain}_i$, where:
$$\text{gain}_i = \text{start\_gain} + \frac{i}{n-1} \times (\text{end\_gain} - \text{start\_gain})$$

If $n = 1$, the gain is simply `start_gain`.

**Real-time constraints:** This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
