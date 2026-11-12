# gpio_command_controller allocates on every update() cycle

Our robot exposes its tool flange IO and a cabinet analog output through
`gpio_controllers/GpioCommandController` (groups `tool_flange_io` and
`cabinet_analog_io`, interfaces like `digital_output_1`). With a malloc hook
armed around `update()`, every cycle performs more than a dozen heap
allocations, roughly 650 bytes, as soon as a command has been received. Both
writing the commands and reading the states into the published state message
contribute.

`update()` runs in the real-time control thread and must not allocate.
Allocating in `on_configure()` or `on_activate()` is fine.

Expected: no heap allocation in `update()` in steady state. Commands must still
be applied to the right interfaces (including when a command lists the
interfaces in a different order), and the published state message must keep
its content.
