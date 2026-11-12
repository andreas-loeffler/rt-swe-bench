# forward_command_controller allocates on every update() cycle

We run a `position_controllers/JointGroupPositionController` (built on
`forward_command_controller`) with 12 joints in a 1 kHz loop. With a malloc
hook armed around the controller's `update()` call, every single cycle
performs a heap allocation, about 96 bytes per cycle, plus one extra
allocation on the first cycle after activation.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: once the controller is active, `update()` does not touch the heap,
and the controller behaves as before: commands from the `~/commands` topic are
forwarded to the command interfaces, NaN means "no command yet", and a command
with the wrong number of entries is rejected.
