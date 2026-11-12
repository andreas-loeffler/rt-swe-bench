# CircularSumWindow

## Description
`CircularSumWindow` maintains a rolling sum of the last `N` samples provided to it. When a new sample is pushed, the oldest sample in the window is removed and the new one is added. If the window is not yet full, it simply sums all samples received so far.

## Public API
- `void configure(size_t window_size)`: Initializes the internal buffer to hold `window_size` elements. Called once before real-time processing.
- `double push(double sample)`: Adds `sample` to the window and returns the current rolling sum of the elements in the window.

## Constraints
- `configure` may allocate memory.
- `push` is a hard real-time method: it must not allocate heap memory, throw exceptions, acquire locks, or perform I/O.

## Behavior
- After `configure(N)`, the initial sum is 0.0.
- For the first `N` calls to `push(x)`, the return value is the sum of all `x` values provided so far.
- From the `N+1`-th call onwards, the return value is the sum of the most recent `N` samples.
