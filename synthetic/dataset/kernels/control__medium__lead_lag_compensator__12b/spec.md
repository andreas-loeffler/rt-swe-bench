# LeadLagCompensator

A real-time lead-lag compensator for signal processing in control systems. It modifies the phase and magnitude of a signal to compensate for time delays (lead) or to provide damping and filtering (lag).

## Public API

### `void configure(double alpha, double beta)`
Configures the compensator coefficients.
- `alpha`: The zero location (lead/lag characteristic).
- `beta`: The pole location (lead/lag characteristic).
- **Preconditions**: $\alpha$ and $\beta$ must be non-zero. 
- **Behavior**: Allocates necessary buffers.

### `double process(double input)`
Processes a single sample through the lead-lag compensator.
- **Preconditions**: `configure` must have been called.
- **Behavior**: Returns the compensated output value. The implementation uses a first-order discrete approximation: $y[n] = \alpha \cdot x[n] + (1 - \alpha - \beta) \cdot y[n-1] + \beta \cdot x[n-1]$.
- **Real-time**: Must not allocate, lock, or throw.

### `void reset()`
Resets the internal state of the compensator to zero.
- **Real-time**: Must not allocate, lock, or throw.
