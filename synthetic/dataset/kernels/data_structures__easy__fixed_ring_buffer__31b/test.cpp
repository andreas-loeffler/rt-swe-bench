#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedRingBuffer<int> k;
    const size_t capacity = 10;
    k.configure(capacity);

    // Test sequence: push 1000 elements
    // We track the expected behavior using a simple std::vector acting as a queue
    std::vector<int> shadow_buffer;

    for (int i = 1; i <= 1000; ++i) {
        int expected_out = 0;
        if (shadow_buffer.size() == capacity) {
            expected_out = shadow_buffer[0];
            shadow_buffer.erase(shadow_buffer.begin());
        }
        shadow_buffer.push_back(i);

        int actual_out = rtk::guarded(run, [&] { 
            return k.push(i); 
        });

        rtk::expect(run, actual_out == expected_out, "Push index " + std::to_string(i));
    }

    return rtk::report(run);
}
