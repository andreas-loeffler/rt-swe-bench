# AlphaBetaFilter Kernel Specification

The `AlphaBetaFilter` is a simple state estimator used to track the position and velocity of a system from noisy position measurements. It uses a first-order lag filter to smooth the position and a second-order filter to estimate the derivative (velocity).

## Public API

### `void configure(double dt, double alpha, double beta)`
- `dt`: The sampling period (time between updates) in seconds.
- `alpha`: The position gain (smoothing factor), typically between 0.0 and 1.0.
- `beta`: The velocity gain, typically between 0.0 and 1.0.
- Precondition: `dt > 0`.

### `double process(double measurement)`
- Updates the filter state based on a new position measurement.
- Returns the estimated current position.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, perform I/O, or use locks.

## Behavior
The filter maintains an internal state for position ($\hat{x}$) and velocity ($\hat{v}$).
For each measurement $z$:
1. Compute the innovation (error): $e = z - \hat{x}_{predicted}$
2. Update position estimate: $\hat{x}_{final} = \hat{x}_{predicted} + \alpha \cdot e$
3. Update velocity estimate: $\hat{v}_{final} = \hat{v}_{previous} + \frac{\beta}{dt} \cdot e$
4. Predict next state: $\hat{x}_{predicted} = \hat{x}_{final} + \hat{v}_{final} \cdot dt$

The filter is initialized to zero on the first call to `process` (or via internal state reset) such that the first measurement sets the initial position.
