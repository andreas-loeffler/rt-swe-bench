# WatchdogSupervisor

The `WatchdogSupervisor` monitors multiple heartbeat signals. For each signal source, it tracks a timeout period. If a source fails to "kick" the watchdog (reset its timer) within its allocated timeout period, the supervisor enters a "failsafe" state.

## Public API

- `void configure(size_t num_sources, const std::vector<double>& timeouts_sec)`
  Configures the supervisor for a specific number of heartbeat sources and their respective timeout durations in seconds. This is called once during initialization and may allocate memory.

- `void kick(size_t source_id)`
  Resets the timer for the specified source. If the system was previously in a failsafe state, this method does NOT clear the failsafe state.
  Precondition: `source_id` must be within the range `[0, num_sources)`.
  Real-time constraint: Must not allocate, throw, lock, or perform I/O.

- `bool process(double delta_time_sec)`
  Advances the internal clock for all monitors by `delta_time_sec`. Returns `true` if the system is currently in a failsafe state (i.e., at least one timer has expired), otherwise returns `false`. Once the system enters a failsafe state, it remains in that state regardless of further `kick` calls.
  Precondition: `delta_time_sec` must be non-negative.
  Real-time constraint: Must not allocate, throw, lock, or perform I/O.

## Behavior
1. Each source starts with a timer equal to its timeout value.
2. `kick(id)` resets the timer for source `id` back to its configured timeout value.
3. `process(dt)` decrements the timers for all sources.
4. If any timer reaches $\le 0$, the supervisor flags a failsafe condition.
5. The failsafe condition is sticky: once `process` returns `true`, it will always return `true` for all subsequent calls.
