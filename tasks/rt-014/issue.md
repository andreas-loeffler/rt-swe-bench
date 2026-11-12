# JTC: undefined behavior when the last trajectory segment is skipped

With `allow_integration_in_goal_trajectories: true`, trajectory points may
carry velocities without positions; the controller deduces the missing
positions while it samples the segment that ends at such a point.

If the last segment is shorter than the controller period, the sample time can
jump from before the second-to-last point straight past the last point. The
last segment is then never entered, so the last point's positions are never
deduced. When the trajectory has played out, `Trajectory::sample()` returns the
last point as the output state, with an empty `positions` vector, and the
controller reads joint positions from it. Depending on the build this
segfaults or commands garbage positions.

## Steps to reproduce

- 3 joints, position command interface, position and velocity state interfaces,
  `allow_integration_in_goal_trajectories: true`, controller at 100 Hz.
- Publish a trajectory with points at 0.100 s and 0.203 s (positions and
  velocities) and a last point at 0.207 s with velocities only (all zero).

## Expected

The controller deduces the last point's positions from the point before it,
as it would have when sampling the last segment, and holds there.

## Actual

After 0.21 s the controller samples a point without positions.
