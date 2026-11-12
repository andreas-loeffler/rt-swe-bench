# diff_drive_controller allocates in update() every few dozen cycles

With a malloc hook armed around `update()` of an active `diff_drive_controller`
that receives a steady stream of `cmd_vel` commands, most cycles are clean, but
about one cycle in 32 performs a 512-byte heap allocation. The allocation is in
the bookkeeping of the speed limiter's previous commands. It shows up with and
without velocity, acceleration or jerk limits configured.

`update()` runs in the real-time control thread and must not allocate, not even
occasionally. Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state, and the same wheel
commands as today, including with acceleration and jerk limits.
