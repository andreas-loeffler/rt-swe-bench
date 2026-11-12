# FirLowpassFilter

The `FirLowpassFilter` class implements a Finite Impulse Response (FIR) digital filter. It performs a discrete-time convolution of an input signal with a fixed set of coefficients.

## Public API

### `void configure(std::vector<float> coefficients)`
Initializes the filter with the provided coefficients. This method is called once before any real-time processing. It is allowed to allocate memory.

### `float process(float sample)`
Processes a single input sample and returns the filtered output. 
- **Precondition**: `configure` must have been called.
- **Real-time Constraint**: This method must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

## Behavior
The output $y[n]$ for a given input $x[n]$ is computed as:
$y[n] = \sum_{i=0}^{N-1} h[i] \cdot x[n-i]$
where $h$ are the coefficients and $N$ is the number of coefficients. The history of samples $x[n-i]$ is maintained internally, initializing to zero.
