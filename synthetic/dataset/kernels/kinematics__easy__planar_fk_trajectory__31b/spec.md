# PlanarForwardKinematics

The `PlanarForwardKinematics` class computes the (x, y) end-effector positions for a two-link planar robotic arm given a sequence of joint angles.

## Public API

### `void configure(size_t max_trajectory_length, double link1_len, double link2_len)`
Prepares the internal buffers for a maximum number of trajectory points and sets the lengths of the two robot links. This method may allocate memory.

### `size_t process(const double* joint_angles, size_t count)`
Computes the forward kinematics for `count` pairs of joint angles.
- **Input**: `joint_angles` is a pointer to an array of doubles. Every two elements `[q1, q2]` represent the angles (in radians) of joint 1 and joint 2.
- **Output**: The end-effector positions are stored in an internal buffer.
- **Return**: The number of points actually processed. If `count` exceeds the value provided in `configure`, it is capped at `max_trajectory_length`.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Mathematical Model
For joint angles $\theta_1, \theta_2$ and link lengths $L_1, L_2$:
$x = L_1 \cos(\theta_1) + L_2 \cos(\theta_1 + \theta_2)$
$y = L_1 \sin(\theta_1) + L_2 \sin(\theta_1 + \theta_2)$

## Preconditions
- `configure` must be called before `process`.
- `joint_angles` must be a valid pointer to at least `2 * std::min(count, max_trajectory_length)` elements.
