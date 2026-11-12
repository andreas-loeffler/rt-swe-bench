# PreAllocatedVector

A container that mimics the behavior of `std::vector<T>` but guarantees zero heap allocations during real-time operations by using a fixed-capacity pre-allocated buffer.

## API

### `void configure(size_t capacity)`
Initializes the internal buffer to a fixed size. This is the only method allowed to allocate memory.

### `bool push_back(const T& value)`
Appends an element to the end of the container. 
- Returns `true` if the element was successfully added.
- Returns `false` if the container is at maximum capacity.

### `T& back()`
Returns a reference to the last element in the container.
- **Precondition**: The container must not be empty.

## Real-time Constraints
- `push_back` and `back` must not allocate heap memory, throw exceptions, use mutexes, or perform I/O.
