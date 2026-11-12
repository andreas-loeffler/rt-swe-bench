# PlanarArmKinematics Spec

The `PlanarArmKinematics` class computes the forward kinematics (FK) and the differential kinematics (Jacobian) for a planar robotic arm with a configurable number of degrees of freedom (DOF).

## API
- `void configure(size_t dof, double* link_lengths)`: Initializes the solver for an arm with `dof` joints and specifies the length of each link. Memory is allocated here to support the real-time path.
- `void solve(const double* joint_positions, double* out_pose)`: Computes the (x, y, theta) pose of the end-effector given the joint angles.
- `void solve_velocity(const double* joint_positions, const double* joint_velocities, double* out_cartesian_vel)`: Computes the linear and angular velocity of the end-effector given joint positions and velocities using the geometric Jacobian.
- `void set_mode(KinematicMode mode)`: Sets the operating mode (e.g., `STANDARD` or `REDUCED_PRECISION` for faster approximation).

## Definitions
- `joint_positions`: Array of size `dof` (radians).
- `out_pose`: Array of size 3: `[x, y, phi]`.
- `joint_velocities`: Array of size `dof` (rad/s).
- `out_cartesian_vel`: Array of size 3: `[vx, vy, omega]`.

## Constraints
- Real-time methods (`solve`, `solve_velocity`, `set_mode`) must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- All memory required for intermediate Jacobian calculations must be pre-allocated in `configure`.
- Precondition: `configure` must be called before any other method.
