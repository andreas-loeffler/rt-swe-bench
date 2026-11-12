# JointVelocityFilter

The `JointVelocityFilter` class implements a simple moving average filter for a stream of joint encoder readings. It smooths the input signal by averaging the most recent $N$ samples.

## API

### `void configure(size_t window_size)`
Initializes the filter with the desired window size $N$. This method allocates the necessary internal memory and is called once during the non-real-time initialization phase.

### `double process(double sample)`
Processes a new encoder reading. 
- **Behavior**: Adds the new sample to the filter, removes the oldest sample if the window is full, and returns the arithmetic mean of the samples currently in the buffer.
- **Preconditions**: `configure` must have been called.
- **Real-time Constraints**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Mathematical Behavior
If the buffer contains $k$ samples $\{x_1, x_2, \dots, x_k\}$ (where $k \le \text{window\_size}$), the output is:
$$\text{output} = \frac{1}{k} \sum_{i=1}^{k} x_i$$
