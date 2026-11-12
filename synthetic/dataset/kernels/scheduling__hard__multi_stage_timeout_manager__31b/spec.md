# MultiStageTimeoutManager

The `MultiStageTimeoutManager` tracks multiple concurrent timers, each associated with a specific ID. Each timer progresses through a series of escalating warning levels based on configurable time thresholds.

## Public API

### Configuration
- `void configure(size_t max_timers, const std::vector<double>& thresholds)`
  - `max_timers`: Maximum number of concurrent timers to track.
  - `thresholds`: A sorted list of time durations. If a timer's elapsed time exceeds `thresholds[i]`, it transitions to stage `i + 1`.

### Real-Time Methods
All real-time methods must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.

- `void reset(uint32_t id)`
  - Resets the timer associated with `id` to 0. If the `id` is not currently tracked, it starts tracking it. If `max_timers` is reached, the call is ignored.
- `void update(uint32_t id, double delta_time)`
  - Increments the elapsed time for timer `id` by `delta_time`.
- `uint8_t get_stage(uint32_t id) const`
  - Returns the current stage of the timer. Stage 0 means no threshold reached. Stage $N$ means the $N$-th threshold has been exceeded. Returns 0 if the ID is not tracked.
- `bool is_critical(uint32_t id) const`
  - Returns true if the timer for `id` has reached the final stage defined in `configure`.

## Behavior
1. A timer starts at stage 0.
2. As `update` is called, the accumulated time is compared against the `thresholds` array.
3. If `accumulated_time >= thresholds[0]`, the stage becomes 1.
4. If `accumulated_time >= thresholds[k]`, the stage becomes $k+1$.
5. `reset` returns the timer to stage 0 and time 0.
6. The manager supports up to `max_timers` unique IDs.
