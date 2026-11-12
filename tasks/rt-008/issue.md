# Open-loop odometry of diff_drive_controller ignores the speed limits

With `open_loop: true` and acceleration (or jerk) limits configured,
`diff_drive_controller` reports odometry that runs ahead of the robot. When a
step command arrives, the wheels ramp up as the limits allow, but the odometry
integrates the full commanded velocity from the first cycle, so the estimated
pose drifts away from where the robot actually is.

Expected: in open loop, the odometry integrates the same, limited velocity
that is sent to the wheels.
