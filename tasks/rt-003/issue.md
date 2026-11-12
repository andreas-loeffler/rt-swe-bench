# Steering controllers allocate in update() when computing wheel commands

The steering controllers (`ackermann_steering_controller`,
`bicycle_steering_controller`, `tricycle_steering_controller`, all built on
`steering_controllers_library`) allocate on the heap in every `update()` cycle
while a velocity reference is active. With an Ackermann setup we see four
allocations per cycle, about 64 bytes, in the inverse kinematics that turns
the twist reference into traction and steering commands.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state. The wheel commands
must stay exactly the same as today, and the existing public kinematics API
(including `SteeringKinematics::get_commands` as used by the tests) must keep
working.
