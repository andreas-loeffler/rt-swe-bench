# PeakHoldAnalyzer

The `PeakHoldAnalyzer` tracks the maximum and minimum peaks of a telemetry signal over a sliding time window (decay period). It ensures that values stay within defined saturation limits and that the peaks decay linearly over time to reflect the most recent signal extremes.

## Public API

### `void configure(float decay_time, float min_sat, float max_sat)`
- `decay_time`: The time (in seconds) it takes for a peak to decay back to the current signal level if no higher/lower peak occurs.
- `min_sat`: The hard lower saturation limit.
- `max_sat`: The hard upper saturation limit.
- Called once during initialization. May allocate memory.

### `void process(float sample, float dt)`
- `sample`: The current signal value.
- `dt`: Time elapsed since the last call (in seconds).
- Updates the internal peak holders.
- **Real-time constraints**: Must not allocate heap memory, throw, lock, or perform I/O.

### `struct AnalysisResult { float max_peak; float min_peak; };`
### `AnalysisResult get_peaks() const`
- Returns the current tracked maximum and minimum peaks.
- **Real-time constraints**: Must not allocate heap memory, throw, lock, or perform I/O.

## Behavior
1. **Saturation**: Every input `sample` is clamped to `[min_sat, max_sat]`.
2. **Peak Tracking**: 
   - If the clamped sample is greater than the current `max_peak`, `max_peak` is updated to the sample and the decay timer is reset.
   - If the clamped sample is lower than the current `min_peak`, `min_peak` is updated to the sample and the decay timer is reset.
3. **Decay**: 
   - The `max_peak` decays linearly towards the current clamped sample over the `decay_time`.
   - The `min_peak` decays linearly towards the current clamped sample over the `decay_time`.
   - If the current sample is the peak, no decay occurs.
