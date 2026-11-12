# ring_buffer

Implement `RingBuffer` in `kernel.hpp`: a first-in, first-out buffer of doubles
with the capacity given to `configure()`. `push(x)` appends and returns false
without changing anything when the buffer is full; `pop(out)` removes the oldest
element into `out` and returns false when the buffer is empty; `size()` returns
the number of stored elements.

`push()` and `pop()` run on a hard real-time path: they must not allocate heap
memory. `configure()` may.
