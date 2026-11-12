# PeakDetector

The `PeakDetector` class identifies local maxima (peaks) in a stream of real-time signal samples. A sample is considered a peak if it is strictly greater than its immediate neighbors and exceeds a user-defined threshold.

## Public API

### `void configure(float threshold, uint32_t min_distance)`
Configures the detector. 
- `threshold`: The minimum value a sample must exceed to be considered a candidate.
- `min_distance`: The minimum number of samples that must pass between consecutive detected peaks.
- This method may allocate memory for internal state.

### `bool process(float sample)`
Processes a single sample from the stream.
- Returns `true` if a new peak was detected at this sample.
- Returns `false` otherwise.

## Behavior
1. **Peak Condition**: A peak is detected at index $i$ if:
   - $sample[i] > threshold$
   - $sample[i] > sample[i-1]$
   - $sample[i] > sample[i+1]$ (Note: Since the stream is real-time, the "next" sample is not known. A peak is confirmed at index $i$ when $sample[i+1]$ is observed and is smaller than $sample[i]$).
2. **Distance Constraint**: A peak is only valid if it occurs at least `min_distance` samples after the previous confirmed peak.
3. **Refinement**: Because $sample[i+1]$ is required to confirm $sample[i]$, the `process` method returns `true` at index $i+1$ when the peak at $i$ is confirmed.
4. **Edge Cases**:
   - If the stream ends, any pending peak candidate is discarded.
   - If `min_distance` is 0, peaks can be detected consecutively.
   - If `threshold` is infinity, no peaks are detected.

## Real-time Guarantees
The `process` method is a hard real-time call. It must not allocate heap memory, throw exceptions, use locks, or perform I/O.
