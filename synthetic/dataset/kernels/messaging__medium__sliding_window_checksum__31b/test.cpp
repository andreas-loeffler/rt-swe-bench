#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    SlidingWindowChecksum k;
    const size_t window_size = 10;
    k.configure(window_size);

    // Local truth tracking
    std::vector<uint8_t> stream;
    uint64_t expected_sum = 0;

    auto update_expected = [&](uint8_t b) {
        stream.push_back(b);
        expected_sum += b;
        if (stream.size() > window_size) {
            expected_sum -= stream[stream.size() - window_size - 1];
        }
    };

    // Test 1: Fill up to window size
    for (int i = 0; i < 100; ++i) {
        uint8_t val = static_cast<uint8_t>(i % 256);
        
        rtk::guarded_void(run, [&] { k.push(val); });
        update_expected(val);

        rtk::expect(run, k.current_checksum() == expected_sum, "Sum check " + std::to_string(i));
        
        size_t expected_fill = (stream.size() < window_size) ? stream.size() : window_size;
        rtk::expect(run, k.current_window_fill() == expected_fill, "Fill check " + std::to_string(i));
    }

    // Test 2: Wrap around and sustain (sliding phase)
    for (int i = 100; i < 600; ++i) {
        uint8_t val = static_cast<uint8_t>(i % 256);
        
        rtk::guarded_void(run, [&] { k.push(val); });
        update_expected(val);

        rtk::expect(run, k.current_checksum() == expected_sum, "Sliding sum check " + std::to_string(i));
        rtk::expect(run, k.current_window_fill() == window_size, "Full window check " + std::to_string(i));
    }

    return rtk::report(run);
}
