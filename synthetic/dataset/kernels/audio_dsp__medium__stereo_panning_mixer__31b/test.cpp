#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <random>

int main() {
    rtk::setup();
    rtk::Run run;

    StereoMixer mixer;
    const size_t MAX_CH = 8;
    const size_t BLOCK = 128;
    
    mixer.configure(MAX_CH, BLOCK);

    // Prepare test data
    std::vector<std::vector<float>> channel_data(MAX_CH, std::vector<float>(BLOCK));
    std::vector<float> left_out(BLOCK);
    std::vector<float> right_out(BLOCK);
    std::vector<const float*> input_ptrs(MAX_CH);

    for (size_t i = 0; i < MAX_CH; ++i) {
        input_ptrs[i] = channel_data[i].data();
    }

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // We will run 600 iterations to exceed the 500 requirement
    for (int i = 0; i < 600; ++i) {
        size_t active_channels = rng() % (MAX_CH + 1); // 0 to MAX_CH
        size_t samples = rng() % (BLOCK + 1);        // 0 to BLOCK
        
        // Randomly set pan for each channel
        for (size_t ch = 0; ch < MAX_CH; ++ch) {
            float pan = dist(rng);
            rtk::guarded_void(run, [&] {
                mixer.set_pan(ch, pan);
            });
        }

        // Fill buffers with random data
        for (size_t ch = 0; ch < MAX_CH; ++ch) {
            for (size_t s = 0; s < BLOCK; ++s) {
                channel_data[ch][s] = dist(rng);
            }
        }

        rtk::guarded_void(run, [&] {
            mixer.process(input_ptrs.data(), active_channels, left_out.data(), right_out.data(), samples);
        });

        // Verification
        for (size_t s = 0; s < samples; ++s) {
            float expectedL = 0.0f;
            float expectedR = 0.0f;
            for (size_t ch = 0; ch < active_channels; ++ch) {
                // Recalculate constant power pan in test (plain code)
                float pan_val = -2.0f + (float(ch % 10) / 5.0f); // Just need a deterministic value, but wait, set_pan was random.
                // To correctly verify, we need the same pan logic.
                // Since we can't store the random pan outside, we re-run the logic for the expected value.
            }
        }
    }

    // Redoing the loop with stored pans to make the check possible
    std::vector<float> current_pans(MAX_CH, 0.0f);
    for (int i = 0; i < 600; ++i) {
        size_t active_channels = rng() % (MAX_CH + 1);
        size_t samples = rng() % (BLOCK + 1);

        for (size_t ch = 0; ch < MAX_CH; ++ch) {
            float pan = dist(rng);
            current_pans[ch] = pan;
            rtk::guarded_void(run, [&] { mixer.set_pan(ch, pan); });
        }

        for (size_t ch = 0; ch < MAX_CH; ++ch) {
            for (size_t s = 0; s < BLOCK; ++s) channel_data[ch][s] = dist(rng);
        }

        rtk::guarded_void(run, [&] {
            mixer.process(input_ptrs.data(), active_channels, left_out.data(), right_out.data(), samples);
        });

        for (size_t s = 0; s < samples; ++s) {
            float expL = 0.0f;
            float expR = 0.0f;
            for (size_t ch = 0; ch < active_channels; ++ch) {
                float p = std::clamp(current_pans[ch], -1.0f, 1.0f);
                float theta = (p + 1.0f) * (3.14159265358979323846f / 4.0f);
                expL += channel_data[ch][s] * std::cos(theta);
                expR += channel_data[ch][s] * std::sin(theta);
            }
            rtk::expect(run, rtk::near(left_out[s], expL), "L_sample_" + std::to_string(s));
            rtk::expect(run, rtk::near(right_out[s], expR), "R_sample_" + std::to_string(s));
        }
    }

    return rtk::report(run);
}
