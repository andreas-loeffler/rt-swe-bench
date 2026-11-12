# SimpleMixer

The `SimpleMixer` class sums multiple input audio buffers into a single output buffer, applying a fixed master gain to the result.

## Public API

- `void configure(size_t num_channels, size_t buffer_size, float master_gain)`
  Configures the mixer for a specific number of input channels, the size of the audio buffers, and the master gain. This method is called once before any real-time processing and is allowed to allocate memory.

- `void process(const float** inputs, float* output)`
  Sums the audio samples from `num_channels` buffers into the `output` buffer.
  
  **Computation:**
  For each sample index `i` from `0` to `buffer_size - 1`:
  `output[i] = (inputs[0][i] + inputs[1][i] + ... + inputs[num_channels-1][i]) * master_gain`

## Preconditions

- `inputs` must be a pointer to an array of `num_channels` pointers, where each pointer points to a buffer of at least `buffer_size` floats.
- `output` must point to a buffer of at least `buffer_size` floats.
- `process` must only be called after `configure`.

## Real-time Constraints

The `process` method runs on a hard real-time path. It must not allocate heap memory, throw exceptions, use locks, or perform I/O.
