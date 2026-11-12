#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t window_size = 10;
    DeadReckoningPos k;
    k.configure(window_size);

    std::vector<double> velocities;
    double expected_pos = 0.0;

    // Test initial state
    rtk::expect(run, k.get_position() == 0.0, "Initial position is 0");

    // Fill buffer partially
    for (int i = 0; i < 5; ++i) {
        double v = static_cast<double>(i + 1);
        velocities.push_back(v);
        expected_pos += v;
        
        rtk::guarded_void(run, [&] { k.update(v); });
        rtk::expect(run, k.get_position() == expected_pos, "Partial buffer sum");
    }

    // Fill buffer to capacity
    for (int i = 5; i < 10; ++i) {
        double v = static_cast<double>(i + 1);
        velocities.push_back(v);
        expected_pos += v;
        
        rtk::guarded_void(run, [&] { k.update(v); });
        rtk::expect(run, k.get_position() == expected_pos, "Full buffer sum");
    }

    // Rotate buffer (circular behavior)
    // At this point, buffer contains [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    // Next update should remove 1 and add 11.
    for (int i = 10; i < 600; ++i) {
        double v = static_cast<double>(i + 1);
        velocities.push_back(v);
        
        // Manual calculation: Sum of last 10 elements
        expected_pos = 0.0;
        int start_idx = (velocities.size() > (size_t)window_size) 
                        ? (int)velocities.size() - (int)window_size 
                        : 0;
        for(size_t j = start_idx; j < velocities.size(); ++j) {
            expected_pos += velocities[j];
        }

        rtk::guarded_void(run, [&] { k.update(v); });
        rtk::expect(run, k.get_position() == expected_pos, "Circular buffer sum");
    }

    return rtk::report(run);
}
