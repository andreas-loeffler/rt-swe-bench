# AlphaBetaTracker Spec

The `AlphaBetaTracker` is a two-state observer used to estimate the position and velocity of a target from a sequence of noisy position measurements. It implements an adaptive Alpha-Beta filter where the gain coefficients change based on the tracking error to balance between responsiveness (convergence) and noise rejection (steady-state).

## Public API

### `void configure(double dt, double alpha_fast, double beta_fast, double alpha_slow, double beta_slow, double threshold)`
Called once before real-time operation.
- `dt`: Sampling period in seconds.
- `alpha_fast`, `beta_fast`: Gains used when the estimation error is above the threshold.
- `alpha_slow`, `beta_slow`: Gains used when the estimation error is below the threshold.
- `threshold`: The absolute difference between measurement and prediction that triggers the "fast" mode.

### `void push(double measurement)`
Processes a new position measurement. This method updates the internal state (position and velocity) using the adaptive gain logic.
- **Real-time Constraint**: Must not allocate heap memory, throw, or perform I/O.

### `double get_position() const`
Returns the current filtered position estimate.
- **Real-time Constraint**: Must not allocate heap memory, throw, or perform I/O.

### `double get_velocity() const`
Returns the current filtered velocity estimate.
- **Real-time Constraint**: Must not allocate heap memory, throw, or perform I/O.

### `void reset()`
Resets the internal state to zero and marks the tracker as uninitialized.
- **Real-time Constraint**: Must not allocate heap memory, throw, or perform I/O.

## Behavior
1. If the tracker is uninitialized, the first `push(z)` sets position to $z$ and velocity to $0$.
2. For subsequent updates:
   - Predict: $\hat{x}_{pred} = \hat{x}_{prev} + \hat{v}_{prev} \cdot dt$
   - Error: $e = z - \hat{x}_{pred}$
   - Gain Selection: If $|e| > threshold$, use $\alpha_{fast}, \beta_{fast}$. Otherwise, use $\alpha_{slow}, \beta_{slow}$.
   - Correct Position: $\hat{x}_{curr} = \hat{x}_{pred} + \alpha \cdot e$
   - Correct Velocity: $\hat{v}_{curr} = \hat{v}_{prev} + \frac{\beta \cdot e}{dt}$
