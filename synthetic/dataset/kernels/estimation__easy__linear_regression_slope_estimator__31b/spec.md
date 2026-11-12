# LinearRegressionEstimator

Estimates the slope of a signal using a simple linear least-squares fit over a sliding window of the most recent samples.

## Public API
- `void configure(size_t window_size)`: Initializes the internal buffer to hold `window_size` samples. This is the only method allowed to allocate memory.
- `double push(double sample)`: Adds a new sample to the sliding window and returns the current estimated slope.

## Behavior
- The estimator maintains a window of the $N$ most recent samples.
- The slope is calculated by fitting a line $y = mx + c$ to the data points $(x_i, y_i)$, where $x_i$ is the index of the sample within the window ($0 \dots N-1$) and $y_i$ is the sample value.
- The slope formula used is:
  $m = \frac{N\sum(xy) - \sum x \sum y}{N\sum(x^2) - (\sum x)^2}$
- If the window is not yet full, the slope is calculated based on the number of samples currently available.
- If the window contains fewer than 2 samples, the slope is returned as $0.0$.

## Real-Time Constraints
- `push` must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
