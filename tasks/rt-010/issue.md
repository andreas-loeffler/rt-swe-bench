# joint_trajectory_controller allocates in every update() while executing a trajectory

While `joint_trajectory_controller` executes a trajectory (3 joints, position
command interface, trajectory points with positions and velocities), a malloc
hook armed around `update()` reports heap allocations in every single cycle,
about eight allocations of 24 bytes each, coming from sampling the trajectory.
They continue while the controller holds the last point.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine. (Accepting a new
trajectory is a separate matter and out of scope here.)

Expected: no heap allocation in `update()` while a trajectory is executed or
held, and exactly the same commanded positions as today.
