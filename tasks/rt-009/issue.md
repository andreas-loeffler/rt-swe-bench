# diff_drive_controller allocates in update() with namespaced frame ids

In a multi-robot setup our `cmd_vel` messages carry namespaced frame ids such
as `robot_1/base_link`. With a malloc hook armed around `update()`, every cycle
of `diff_drive_controller` then performs a small heap allocation (18 bytes for
this frame id). With a short frame id like `base_link` the same cycles are
clean.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state, for any frame id
length, and the same wheel commands, odometry and command timeout behavior as
today.
