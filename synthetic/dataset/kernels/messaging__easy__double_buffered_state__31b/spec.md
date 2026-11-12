# DoubleBufferedState

The `DoubleBufferedState` kernel provides a mechanism to exchange a state vector between a producer (writer) and a consumer (reader) ensuring that the reader always sees a consistent snapshot of the state without blocking the writer.

## Public API

### `void configure(size_t size)`
- Precondition: Called once before any real-time calls.
- Behavior: Allocates internal buffers to hold two copies of a state vector of the specified `size`.

### `void write(const double* data)`
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.
- Behavior: Copies the provided `data` into the current write-buffer.

### `void swap()`
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.
- Behavior: Swaps the roles of the read-buffer and write-buffer. The last completed `write` becomes available to the reader.

### `void read(double* output)`
- Real-time constraint: Must not allocate, throw, lock, or perform I/O.
- Behavior: Copies the current read-buffer into the `output` array.

## Invariants
- The reader always accesses a stable snapshot.
- Data written after a `swap()` is not visible to `read()` until the next `swap()`.
