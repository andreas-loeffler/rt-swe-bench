# FixedCapacityStack

A LIFO (Last-In, First-Out) stack implementation designed for hard real-time contexts. The stack uses a pre-allocated buffer to ensure that no dynamic memory allocation occurs during push and pop operations.

## Public API

### `void configure(size_t capacity)`
Initializes the stack with a maximum capacity. This method is called once during the setup phase and is permitted to allocate heap memory.

### `bool push(int value)`
Pushes an integer onto the stack.
- **Returns**: `true` if the value was successfully pushed; `false` if the stack is full.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `bool pop(int& out_value)`
Pops the top integer from the stack and assigns it to `out_value`.
- **Returns**: `true` if a value was popped; `false` if the stack was empty.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

### `size_t size() const`
Returns the current number of elements in the stack.
- **Real-time Constraint**: Must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Preconditions
- `configure(capacity)` must be called before any other method.
- The `capacity` provided to `configure` must be greater than 0.
