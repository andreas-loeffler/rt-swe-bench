#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <numbers>

int main() {
    rtk::setup();
    rtk::Run run;

    WindowedBuffer k;
    const size_t FRAME_SIZE = 64;
    k.configure(FRAME_SIZE);

    // Independent verification data
    std::vector<float> window(FRAME_SIZE);
    for (size_t i = 0; i < FRAME_SIZE; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * i / (FRAME_SIZE - 1)));
    }

    // Test sequence: Use non-trivial values to catch *= vs /= and index errors
    // 1. First frame
    for (size_t i = 0; i < FRAME_SIZE; ++i) {
        float val = static_cast<float>(i + 1);
        bool ready = rtk::guarded(run, [&]() {
            return k.push(val);
        });

        if (i < FRAME_SIZE - 1) {
            rtk::expect(run, ready == false, "Frame not ready yet");
        } else {
            rtk::expect(run, ready == true, "Frame ready at boundary");
            const float* frame = rtk::guarded(run, [&]() {
                return k.get_frame();
            });
            for (size_t j = 0; j < FRAME_SIZE; ++j) {
                float expected = static_cast<float>(j + 1) * window[j];
                rtk::expect(run, rtk::near(frame[j], expected, 1e-6), "Frame content match");
            }
        }
    }

    // 2. Second frame with different values to ensure buffer is cleared/overwritten correctly
    for (size_t i = 0; i < FRAME_SIZE; ++i) {
        float val = 100.0f + i;
        bool ready = rtk::guarded(run, [&]() {
            return k.push(val);
        });

        if (i == FRAME_SIZE - 1) {
            rtk::expect(run, ready == true, "Second frame ready");
            const float* frame = rtk::guarded(run, [&]() {
                return k.get_frame();
            });
            for (size_t j = 0; j < FRAME_SIZE; ++j) {
                float expected = (100.0f + j) * window[j];
                rtk::expect(run, rtk::near(frame[j], expected, 1e-6), "Second frame content match");
            }
        }
    }

    // 3. Verify frame_size = 1 edge case
    WindowedBuffer k_small;
    k_small.configure(1);
    bool ready_small = rtk::guarded(run, [&]() {
        return k_small.push(5.0f);
    });
    rtk::expect(run, ready_small == true, "Size 1 ready");
    const float* frame_small = rtk::guarded(run, [&]() {
        return k_small.get_frame();
    });
    rtk::expect(run, rtk::near(*frame_small, 5.0f, 1e-6), "Size 1 value match");

    return rtk::report(run);
}
