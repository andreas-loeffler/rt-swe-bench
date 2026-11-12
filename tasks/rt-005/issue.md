# omni_wheel_drive_controller odometry allocates on every update() cycle

With closed-loop odometry (`open_loop: false`, wheel feedback from the
hardware), a malloc hook armed around `update()` reports about 28 heap
allocations, roughly 2 KB, in every cycle, mostly in the odometry that turns
the wheel speeds into the robot velocity. It happens with position and with
velocity feedback.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state. The odometry must
give the same results as today (pose and velocities, to numerical precision),
and the wheel commands must not change.
