# DeadReckoningPos

This kernel estimates a robot's current position by integrating a sequence of velocity measurements over a fixed historical window.

## API

### `configure(size_t window_size)`
Configures the kernel with the number of previous velocity measurements to consider.
- **Preconditions**: `window_size > 0`.
- **Side Effects**: Allocates memory proportional to `window_size`.

### `update(double velocity)`
Adds a new velocity measurement to the history. If the history is full, the oldest measurement is discarded.
- **Returns**: `void`
- **Real-time**: Must not allocate, lock, or perform I/O.

### `get_position()`
Calculates the current estimated position. The position is the sum of all velocities currently stored in the history buffer.
- **Returns**: `double`
- **Real-time**: Must not allocate, lock, or perform I/O.

## Behavior
The kernel maintains a circular buffer of velocities. `update` pushes a new velocity into the buffer. `get_position` returns the sum of all elements currently in the buffer. If the buffer is not full yet (e.g., after fewer than `window_size` updates), only the received velocities are summed.
