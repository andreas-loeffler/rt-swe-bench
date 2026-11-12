# omni_wheel_drive_controller allocates in update() when publishing odometry TF

Running `omni_wheel_drive_controller` with `open_loop: true` and
`enable_odom_tf: true` (the default), a malloc hook armed around `update()`
reports a heap allocation in every cycle, in the code that publishes the odometry
transform.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state. The controller must
keep publishing the same odometry and the same TF (one transform from the odom
frame to the base frame) as today.
