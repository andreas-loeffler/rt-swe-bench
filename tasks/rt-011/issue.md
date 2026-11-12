# pid_controller allocates in update() when a gain is changed at runtime

We tune the gains of a running `pid_controller` with `ros2 param set` (for
example `gains.joint1.p`). With a malloc hook armed around `update()`, the
cycle right after each parameter change performs several heap allocations,
and the update also takes a blocking lock on the parameter listener every
cycle. Cycles without a parameter change are clean in chained mode.

`update()` runs in the real-time control thread and must not allocate or
block. Allocating in `on_configure()` or `on_activate()` is fine.

Expected: changing a parameter at runtime takes effect as today (the next
cycles use the new gain), without heap allocation in `update()`.
