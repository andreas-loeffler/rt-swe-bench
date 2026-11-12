#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedSizeQueue<int> queue;
    const size_t capacity = 10;
    queue.configure(capacity);

    std::vector<int> shadow_queue;

    // Test 1: Fill to capacity
    for (int i = 0; i < 1000; ++i) {
        int val = i;
        bool success = rtk::guarded(run, [&] { return queue.push(val); });
        
        if (shadow_queue.size() < capacity) {
            shadow_queue.push_back(val);
            rtk::expect(run, success == true, "Push within capacity " + std::to_string(i));
        } else {
            rtk::expect(run, success == false, "Push overflow " + std::to_string(i));
        }
    }

    // Test 2: Drain and Refill (Wrap-around)
    for (int i = 0; i < 1000; ++i) {
        int popped_val = -1;
        bool success = rtk::guarded(run, [&] { return queue.pop(popped_val); });

        if (!shadow_queue.empty()) {
            int expected = shadow_queue.front();
            shadow_queue.erase(shadow_queue.begin());
            rtk::expect(run, success == true, "Pop success " + std::to_string(i));
            rtk::expect(run, popped_val == expected, "Pop value " + std::to_string(i));
        } else {
            rtk::expect(run, success == false, "Pop underflow " + std::to_string(i));
        }

        // Interleave a push to force index wrap-around
        int push_val = i + 10000;
        bool push_success = rtk::guarded(run, [&] { return queue.push(push_val); });
        if (push_success) {
            shadow_queue.push_back(push_val);
        }
        rtk::expect(run, push_success == (shadow_queue.size() <= capacity), "Interleave push " + std::to_string(i));
    }

    // Test 3: Exhaustion
    int final_pop = -1;
    while (rtk::guarded(run, [&] { return queue.pop(final_pop); })) {
        shadow_queue.erase(shadow_queue.begin());
    }
    rtk::expect(run, shadow_queue.empty(), "Queue should be empty");

    return rtk::report(run);
}
