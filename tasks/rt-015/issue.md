# forward_command_controller: NaN entries are written to the hardware

The controller treats a command that is NaN for every joint as "no command
received yet" and leaves the command interfaces alone. A command that is NaN
for only some joints, however, is forwarded as is: those joints receive NaN.

We send partial commands from a teleoperation node that only knows a subset of
the joints and marks the others with NaN. The joints marked NaN should keep
their current command, as the all-NaN case suggests, instead of being
commanded NaN.

## Steps to reproduce

1. Activate a `forward_command_controller` with several joints.
2. Publish a command with a value for every joint.
3. Publish a command with NaN for some of the joints.

## Expected

The joints with NaN keep the value from step 2; the others take the new value.

## Actual

The joints with NaN are commanded NaN.
