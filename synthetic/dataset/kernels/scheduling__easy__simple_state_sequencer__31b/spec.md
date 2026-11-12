# StateSequencer

The `StateSequencer` is a real-time state machine that transitions through a predefined sequence of states. Each state in the sequence is assigned a specific duration measured in cycles (calls to the `step` method).

## Public API

- `void configure(const std::vector<uint32_t>& state_durations)`
  Initializes the sequencer with a list of durations. The number of durations defines the number of states (0 to N-1).
- `uint32_t step()`
  Advances the internal cycle counter and returns the index of the current state.

## Behavior

1. The sequencer starts in state `0`.
2. It remains in state `i` for exactly `state_durations[i]` calls to `step()`.
3. Once the duration for state `i` is exhausted, it immediately transitions to state `i + 1`.
4. When the final state in the sequence is exhausted, the sequencer wraps around to state `0`.
5. If `configure` is called with an empty vector, `step()` returns 0.

## Real-time Constraints

- The `step()` method is a hard real-time function.
- It must not allocate heap memory, throw exceptions, lock mutexes, or perform I/O.
- All memory required for operation must be allocated during `configure()`.
