# StereoMixer

The `StereoMixer` takes multiple mono audio sources and mixes them into a single stereo output buffer using constant-power panning.

## Public API

### `void configure(size_t max_channels, size_t block_size)`
Prepares the mixer for a maximum number of input channels and a fixed processing block size. This is the only method allowed to allocate memory.

### `void set_pan(size_t channel_idx, float pan)`
Updates the panning position for a specific channel.
- `pan`: A value from -1.0 (full left) to 1.0 (full right). 0.0 is center.
- This is a real-time method.

### `void process(const float* const* input_buffers, size_t num_channels, float* left_out, float* right_out, size_t num_samples)`
Mixes `num_channels` mono buffers into the stereo output.
- `input_buffers`: An array of pointers to mono buffers.
- `num_samples`: Number of samples to process (must be $\le$ `block_size`).
- Panning Law: Constant-power panning is used.
  - $\theta = \frac{\text{pan} + 1}{2} \times \frac{\pi}{2}$
  - $gain_L = \cos(\theta)$
  - $gain_R = \sin(\theta)$
- This is a real-time method.

## Constraints
- `process` and `set_pan` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- `num_channels` in `process` must be $\le$ `max_channels` configured.
- `num_samples` in `process` must be $\le$ `block_size` configured.
