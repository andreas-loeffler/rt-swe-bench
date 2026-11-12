# CascadedBiquadIIR

The `CascadedBiquadIIR` class implements a digital filter composed of a series of second-order sections (Biquads) connected in cascade. It is used to create high-order filters with steep slopes while maintaining numerical stability.

## API

### `void configure(size_t num_sections, float sample_rate)`
Configures the filter.
- `num_sections`: The number of Biquad sections to cascade.
- `sample_rate`: The operating sample rate in Hz.
- This method is the only place where heap allocation is permitted.

### `void set_coefficients(size_t section_idx, float b0, float b1, float b2, float a1, float a2)`
Sets the coefficients for a specific Biquad section.
- Uses the Direct Form II Transposed structure.
- The transfer function for one section is: $H(z) = \frac{b_0 + b_1 z^{-1} + b_2 z^{-2}}{1 + a_1 z^{-1} + a_2 z^{-2}}$.
- Precondition: `section_idx` must be within $[0, \text{num\_sections})$.

### `float process(float input)`
Processes a single audio sample through the entire cascade of Biquads.
- Returns the filtered sample.
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.

### `void reset()`
Resets the internal state (delay registers) of all sections to zero.
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.

### `bool is_stable()`
Checks if the current coefficients of all sections satisfy the basic stability criteria for a Biquad (poles inside the unit circle).
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.
