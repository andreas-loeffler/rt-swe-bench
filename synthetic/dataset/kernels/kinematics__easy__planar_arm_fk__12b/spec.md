# PlanarArmKinematics

Computes the (x, y) end-effector position of a 2D planar robotic arm with $N$ revolute joints.

## API
### `void configure(size_t num_joints)`
Pre-allocates internal buffers for a robot arm with a specific number of joints.

### `std::pair<double, double> process(const std::vector<double>& joint_angles)`
Computes the $(x, y)$ coordinates of the end-effector given a vector of joint angles (in radians).
- **Precondition**: `joint_angles.size()` must match the size configured in `configure`.
- **Constraints**: Must be hard real-time (no heap allocation, no locks, no I/O, no exceptions).

## Behavior
The end-effector position is calculated using forward kinematics. Each joint $i$ has a length $L_i$. For this kernel, $L_i$ is assumed to be $1.0$ for all joints.
The position $(x, y)$ is calculated as:
$x = \sum_{i=0}^{n-1} \cos(\sum_{j=0}^{i} \theta_j)$
$y = \sum_{i=0}^{n-1} \sin(\sum_{j=0}^{i} \theta_j)$
where $\theta$ is the angle of joint $i$.
