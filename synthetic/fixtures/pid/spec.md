# pid

Implement `Pid::process()` in `kernel.hpp`: one PID step for each of `dofs`
axes. For axis i with error e = reference[i] - measured[i]: the integral term
accumulates ki * e * dt and is clamped to [-i_limit, i_limit] after each update;
the derivative term is (e - previous e) / dt, and 0 on the first call; the
command is kp * e + integral + kd * derivative. `configure()` sets the gains,
the time step, the integral limit and the number of axes.

`process()` runs in a hard real-time control loop: it must not allocate heap
memory. `configure()` may.
