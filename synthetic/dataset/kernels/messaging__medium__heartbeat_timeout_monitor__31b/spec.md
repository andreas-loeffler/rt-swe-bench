# HeartbeatTimeoutMonitor

The `HeartbeatTimeoutMonitor` tracks the health of multiple communication channels by monitoring the arrival of heartbeat messages. It detects if any specific channel or any channel in general has failed to send a message within a specified timeout period.

## Public API

### `void configure(size_t num_channels, double timeout_seconds)`
- **Preconditions**: Called once before any real-time calls.
- **Behavior**: Reserves memory to track `num_channels` channels and sets the timeout threshold.

### `void update(size_t channel_id, double timestamp)`
- **Preconditions**: `channel_id` must be within `[0, num_channels)`.
- **Behavior**: Updates the last seen timestamp for the specified channel.
- **Real-time constraint**: Must not allocate heap memory, throw, lock, or perform I/O.

### `bool is_channel_timed_out(size_t channel_id, double current_time)`
- **Preconditions**: `channel_id` must be within `[0, num_channels)`.
- **Behavior**: Returns `true` if the difference between `current_time` and the last seen timestamp for `channel_id` exceeds `timeout_seconds`. If the channel has never received a message, it is considered timed out.
- **Real-time constraint**: Must not allocate heap memory, throw, lock, or perform I/O.

### `bool any_timed_out(double current_time)`
- **Preconditions**: None.
- **Behavior**: Returns `true` if at least one channel is currently timed out (including channels that have never received a message).
- **Real-time constraint**: Must not allocate heap memory, throw, lock, or perform I/O.
