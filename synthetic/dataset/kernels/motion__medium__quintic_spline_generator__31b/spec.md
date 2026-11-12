# QuinticSplineGenerator

The `QuinticSplineGenerator` computes a quintic polynomial trajectory between two points in a single dimension. This ensures smooth motion by guaranteeing continuity in position, velocity, and acceleration.

## API
- `void configure(double duration)`: Sets the total time duration for the move.
- `void set_targets(double start_pos, double end_pos)`: Sets the boundary conditions.
- `State step(double current_time)`: Computes the position, velocity, and acceleration at the given time.

## Data Types
```cpp
struct State {
    double position;
    double velocity;
    double acceleration;
};
```

## Behavior
- Given a duration $T$, a start position $P_0$, and an end position $P_1$.
- Boundary conditions at $t=0$: $pos = P_0, vel = 0, acc = 0$.
- Boundary conditions at $t=T$: $pos = P_1, vel = 0, acc = 0$.
- For $t \le 0$, the state is clamped to the start boundary.
- For $t \ge T$, the state is clamped to the end boundary.
- The trajectory follows the polynomial: $s(t) = P_0 + (P_1 - P_0) \cdot (10(t/T)^3 - 15(t/T)^4 + 6(t/T)^5)$.

## Real-Time Constraints
- `set_targets` and `step` are real-time methods.
- They must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
