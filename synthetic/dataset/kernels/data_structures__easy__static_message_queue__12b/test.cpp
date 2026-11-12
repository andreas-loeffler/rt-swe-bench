#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

struct Message {
    int id;
    float value;
};

int main() {
    rtk::setup();
    rtk::Run run;

    StaticMessageQueue<Message> queue;
    const size_t capacity = 10;
    queue.configure(capacity);

    // Test 1: Push to empty, pop until empty
    for (int i = 0; i < 500; ++i) {
        // Every 10 iterations, we do a complex sequence to hit various states
        int cycle = i % 10;
        
        if (cycle < 5) { // Filling part
            bool success = rtk::guarded(run, [&] { return queue.push({i, (float)i}); });
            // Expected: Success until we hit capacity
            // Since we reset or cycle, we calculate expectation:
            // But for a simple test, let's just ensure it behaves.
            rtk::expect(run, success == true, "Push should succeed when space exists");
        } else { // Popping part
            Message m;
            bool success = rtk::guarded(run, [&] { return queue.pop(m); });
            // This might be false if we didn't push enough, but we are cycling.
            // Let's use a more deterministic path:
        }
    }

    // Deterministic Test 2: Fill to capacity and overflow
    // Clear queue first (manually by popping)
    Message m;
    while(rtk::guarded(run, [&] { return queue.pop(m); }));

    for (int i = 0; i < capacity; ++i) {
        bool success = rtk::guarded(run, [&] { return queue.push({i, 0.0f}); });
        rtk::expect(run, success == true, "Push success within capacity");
    }

    // Should fail now
    bool overflow = rtk::guarded(run, [&] { return queue.push({999, 999.0f}); });
    rtk::expect(run, overflow == false, "Push fails at capacity");

    // Pop all
    for (int i = 0; i < capacity; ++i) {
        bool success = rtk::guarded(run, [&] { return queue.pop(m); });
        rtk::expect(run, success == true, "Pop success within capacity");
        rtk::expect(run, m.id == i, "Correct ID popped");
    }

    // Should fail now (empty)
    bool empty_pop = rtk::guarded(run, [&] { return queue.pop(m); });
    rtk::expect(run, empty_pop == false, "Pop fails on empty");

    // Wrap around check
    for (int i = 0; i < capacity; ++i) {
        rtk::guarded(run, [&] { return queue.push({i + 1000, 0.0f}); });
    }
    for (int i = 0; i < capacity; ++i) {
        Message m2;
        rtk::guarded(run, [&] { return queue.pop(m2); });
        rtk::expect(run, m2.id == i + 1000, "Wrap around ID check");
    }

    return rtk::report(run);
}
