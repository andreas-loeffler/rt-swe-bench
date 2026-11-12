# pid_controller allocates in update() while running from the reference topic

With `pid_controller` in non-chained mode (references arrive on
`~/reference`), a malloc hook armed around `update()` reports heap allocations
on every cycle: three allocations, about 48 bytes per cycle for a single DoF,
and more for more DoFs. This happens even when no new reference arrives
between cycles.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state, with the controller
behaving as before (a received reference is applied once to the reference
interfaces, and the PID output is computed and written as today).
