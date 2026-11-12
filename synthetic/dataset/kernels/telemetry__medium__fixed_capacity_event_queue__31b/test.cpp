#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedCapacityEventQueue queue;
    const size_t CAPACITY = 100;
    queue.configure(CAPACITY);

    // Test Case 1: Fill to capacity and then saturate
    for (uint32_t i = 0; i < CAPACITY; ++i) {
        TelemetryEvent ev{i, static_cast<double>(i), static_cast<float>(i) * 1.1f};
        bool success = rtk::guarded(run, [&] { return queue.push(ev); });
        rtk::expect(run, success, "Push within capacity " + std::to_string(i));
    }

    // The 101st push should fail
    TelemetryEvent overflow_ev{999, 0.0, 0.0};
    bool overflow_success = rtk::guarded(run, [&] { return queue.push(overflow_ev); });
    rtk::expect(run, !overflow_success, "Queue saturation check");

    // Test Case 2: Empty the queue and check values
    for (uint32_t i = 0; i < CAPACITY; ++i) {
        TelemetryEvent popped_ev{};
        bool success = rtk::guarded(run, [&] { return queue.pop(popped_ev); });
        rtk::expect(run, success, "Pop within capacity " + std::to_string(i));
        rtk::expect(run, popped_ev.id == i, "Pop value match " + std::to_string(i));
        rtk::near(run, popped_ev.value, static_cast<float>(i) * 1.1f, 1e-5f);
    }

    // The next pop should fail
    TelemetryEvent empty_ev{};
    bool empty_success = rtk::guarded(run, [&] { return queue.pop(empty_ev); });
    rtk::expect(run, !empty_success, "Queue empty check");

    // Test Case 3: Rapid wrap-around (Producer/Consumer interleaved)
    // We will do 500 iterations of push/pop to exercise the ring wrap-around
    for (uint32_t i = 0; i < 500; ++i) {
        TelemetryEvent push_ev{i, static_cast<double>(i), 1.0f};
        rtk::guarded_void(run, [&] { queue.push(push_ev); });

        TelemetryEvent pop_ev{};
        bool success = rtk::guarded(run, [&] { return queue.pop(pop_ev); });
        rtk::expect(run, success, "Interleaved pop success " + std::to_string(i));
        rtk::expect(run, pop_ev.id == i, "Interleaved pop ID " + std::to_string(i));
    }

    // Test Case 4: Partial fill and burst pop
    const size_t BURST_SIZE = 20;
    for (uint32_t i = 0; i < BURST_SIZE; ++i) {
        TelemetryEvent ev{i + 1000, 0.0, 0.0};
        rtk::guarded_void(run, [&] { queue.push(ev); });
    }

    for (uint32_t i = 0; i < BURST_SIZE; ++i) {
        TelemetryEvent pop_ev{};
        bool success = rtk::guarded(run, [&] { return queue.pop(pop_ev); });
        rtk::expect(run, success, "Burst pop success " + std::to_string(i));
        rtk::expect(run, pop_ev.id == (i + 1000), "Burst pop ID match");
    }

    return rtk::report(run);
}
