# Steering controllers: odometry becomes NaN after a non-finite velocity reference

With `open_loop: true`, a single `cmd_vel` message with a non-finite angular
velocity (we saw `inf` from a misbehaving teleop node) breaks the steering
controllers built on `steering_controllers_library`: the odometry turns into
`nan`/`inf` and never recovers, and the wheel and steering commands are no
longer finite either. A NaN linear velocity has a similar effect on the
odometry.

Expected: non-finite references are ignored. The commands stay finite, and the
open-loop odometry treats a non-finite reference as zero velocity instead of
integrating it.
