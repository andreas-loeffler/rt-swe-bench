# StaticMessageQueue

A thread-safe, fixed-capacity, first-in-first-out (FIFO) queue for POD (Plain Old Data) types. This structure is designed for high-frequency inter-thread communication in robotic control loops.

## API

### `template <typename T> void configure(size_t capacity)`
Initializes the queue with a fixed capacity. 
- **Preconditions**: `capacity > 0`.
- **Behavior**: Allocates internal memory for the circular buffer. This is the only method allowed to allocate memory.

### `template <typename T> bool push(const T& item)`
Inserts an item into the back of the queue.
- **Preconditions**: None.
- **Behavior**: Returns `true` if the item was successfully enqueued, `false` if the queue is full.

### `template <typename T> bool pop(T& out_item)`
Removes and returns the item at the front of the queue.
- **Preconditions**: None.
- **Behavior**: Returns `true` if an item was retrieved, `false` if the queue is empty. The `out_item` is overwritten by the popped value.

## Real-time Constraints
The methods `push` and `pop` are real-time and must not perform heap allocation, throw exceptions, use mutexes, or perform I/O. They use a spinlock or atomic operations to ensure thread safety.

## Behavior
- The queue is circular.
- If `push` is called on a full queue, the operation fails immediately (no blocking).
- If `pop` is called on an empty queue, the operation fails immediately (no blocking).
