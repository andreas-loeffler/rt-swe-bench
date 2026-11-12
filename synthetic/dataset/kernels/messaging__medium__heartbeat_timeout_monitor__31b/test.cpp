#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <limits>

int main() {
    rtk::setup();
    rtk::Run run;

    HeartbeatTimeoutMonitor k;
    const size_t num_channels = 5;
    const double timeout = 1.0;
    k.configure(num_channels, timeout);

    // Internal tracking for expected values
    std::vector<double> expected_last_seen(num_channels, -std::numeric_limits<double>::infinity());
    auto check_channel = [&](size_t id, double now) {
        return (now - expected_last_seen[id]) > timeout;
    };
    auto check_any = [&](double now) {
        for(double ts : expected_last_seen) if((now - ts) > timeout) return true;
        return false;
    };

    double current_time = 0.0;

    // Test 1: Initial state (all should be timed out)
    for (size_t i = 0; i < num_channels; ++i) {
        bool result = rtk::guarded(run, [&] { return k.is_channel_timed_out(i, current_time); });
        rtk::expect(run, result == check_channel(i, current_time), "initial_channel_timeout");
    }
    rtk::expect(run, rtk::guarded(run, [&] { return k.any_timed_out(current_time); }) == check_any(current_time), "initial_any_timeout");

    // Test 2: Iterative updates and queries
    for (int step = 0; step < 600; ++step) {
        current_time += 0.1;
        
        // Update a rotating channel
        size_t target_channel = step % num_channels;
        rtk::guarded_void(run, [&] { k.update(target_channel, current_time); });
        expected_last_seen[target_channel] = current_time;

        // Check a specific channel
        size_t query_channel = (step + 2) % num_channels;
        bool res_chan = rtk::guarded(run, [&] { return k.is_channel_timed_out(query_channel, current_time); });
        rtk::expect(run, res_chan == check_channel(query_channel, current_time), "step_channel_timeout");

        // Check global state
        bool res_any = rtk::guarded(run, [&] { return k.any_timed_out(current_time); });
        rtk::expect(run, res_any == check_any(current_time), "step_any_timeout");
    }

    // Test 3: Boundary conditions
    double sync_time = 100.0;
    for (size_t i = 0; i < num_channels; ++i) {
        rtk::guarded_void(run, [&] { k.update(i, sync_time); });
        expected_last_seen[i] = sync_time;
    }
    
    // Exactly at timeout boundary (current_time - last_seen == timeout)
    // Spec: "exceeds timeout_seconds" means >.
    // If (current_time - last_seen) == timeout, it does NOT exceed.
    rtk::expect(run, rtk::guarded(run, [&] { return k.is_channel_timed_out(0, sync_time + timeout); }) == false, "boundary_channel_timeout_false");
    rtk::expect(run, rtk::guarded(run, [&] { return k.any_timed_out(sync_time + timeout); }) == false, "boundary_any_timeout_false");
    
    // Just barely past timeout
    rtk::expect(run, rtk::guarded(run, [&] { return k.is_channel_timed_out(0, sync_time + timeout + 1e-9); }) == true, "past_boundary_channel_timeout_true");
    rtk::expect(run, rtk::guarded(run, [&] { return k.any_timed_out(sync_time + timeout + 1e-9); }) == true, "past_boundary_any_timeout_true");

    // Test 4: Out of bounds channel_id (referenced in reference.hpp logic)
    // Spec says precondition is [0, num_channels), but we check that the implementation handles it 
    // correctly as per the reference's attempt to be robust.
    rtk::expect(run, rtk::guarded(run, [&] { return k.is_channel_timed_out(num_channels + 10, sync_time); }) == true, "oob_channel_timeout");

    return rtk::report(run);
}
