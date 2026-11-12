#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;
    RingBufferIntegrator k;
    
    const size_t WINDOW_SIZE = 10;
    k.configure(WINDOW_SIZE);

    // Reference state for local verification
    std::vector<float> ref_buffer;
    float ref_sum = 0.0f;

    auto update_ref = [&](float val) {
        if (ref_buffer.size() < WINDOW_SIZE) {
            ref_buffer.push_back(val);
            ref_sum += val;
        } else {
            float oldest = ref_buffer[0];
            ref_sum -= oldest;
            ref_buffer.erase(ref_buffer.begin());
            ref_buffer.push_back(val);
            ref_sum += val;
        }
    };

    // 1. Test filling the buffer (underflow/initial state)
    for (int i = 0; i < 15; ++i) {
        float val = static_cast<float>(i + 1);
        rtk::guarded_void(run, [&] { k.push(val); });
        update_ref(val);
        
        float result = rtk::guarded(run, [&] { return k.get_sum(); });
        rtk::expect(run, rtk::near(result, ref_sum), "Fill phase sum " + std::to_string(i));
    }

    // 2. Test wrap-around and saturation
    for (int i = 0; i < 500; ++i) {
        float val = static_cast<float>(i % 100);
        rtk::guarded_void(run, [&] { k.push(val); });
        update_ref(val);

        float result = rtk::guarded(run, [&] { return k.get_sum(); });
        rtk::expect(run, rtk::near(result, ref_sum), "Steady state sum " + std::to_string(i));
    }

    // 3. Test reset and re-fill
    rtk::guarded_void(run, [&] { k.reset(); });
    ref_buffer.clear();
    ref_sum = 0.0f;

    rtk::expect(run, rtk::near(rtk::guarded(run, [&]{ return k.get_sum(); }), 0.0f), "Post-reset sum");

    for (int i = 0; i < 20; ++i) {
        float val = static_cast<float>(i * 2);
        rtk::guarded_void(run, [&] { k.push(val); });
        update_ref(val);
        
        float result = rtk::guarded(run, [&] { return k.get_sum(); });
        rtk::expect(run, rtk::near(result, ref_sum), "Re-fill phase sum " + std::to_string(i));
    }

    return rtk::report(run);
}
