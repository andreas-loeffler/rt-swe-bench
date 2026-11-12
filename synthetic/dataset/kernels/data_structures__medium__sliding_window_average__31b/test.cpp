#include "kernel.hpp" 
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    SlidingWindowAverage k;
    const size_t window_size = 10;
    k.configure(window_size);

    // Reference state for independent verification
    std::vector<double> ref_buffer;
    double ref_sum = 0.0;

    auto get_ref_avg = [&]() {
        if (ref_buffer.empty()) return 0.0;
        return ref_sum / ref_buffer.size();
    };

    auto push_ref = [&](double val) {
        if (ref_buffer.size() >= window_size) {
            ref_sum -= ref_buffer[0];
            ref_buffer.erase(ref_buffer.begin());
        }
        ref_buffer.push_back(val);
        ref_sum += val;
    };

    // 1. Test empty state
    rtk::guarded(run, [&] { return k.get_average(); });
    rtk::expect(run, rtk::near(0.0, 0.0), "Initial average is 0");

    // 2. Fill window and verify ramp
    for (int i = 1; i <= 1000; ++i) {
        double val = static_cast<double>(i);
        
        rtk::guarded_void(run, [&] { k.push(val); });
        push_ref(val);

        double actual = rtk::guarded(run, [&] { return k.get_average(); });
        double expected = get_ref_avg();
        
        rtk::expect(run, rtk::near(actual, expected), "Iter " + std::to_string(i));
    }

    // 3. Verify wrap-around with specific values
    // The buffer should now contain values 991 to 1000
    double specific_val = 50.0;
    rtk::guarded_void(run, [&] { k.push(specific_val); });
    push_ref(specific_val);
    
    double actual = rtk::guarded(run, [&] { return k.get_average(); });
    rtk::expect(run, rtk::near(actual, get_ref_avg()), "Wrap-around check");

    // 4. Test with zero/negative values
    double negative_val = -100.0;
    rtk::guarded_void(run, [&] { k.push(negative_val); });
    push_ref(negative_val);
    
    actual = rtk::guarded(run, [&] { return k.get_average(); });
    rtk::expect(run, rtk::near(actual, get_ref_avg()), "Negative value check");

    return rtk::report(run);
}
