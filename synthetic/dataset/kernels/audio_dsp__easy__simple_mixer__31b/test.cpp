#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <random>

int main() {
    rtk::setup();
    rtk::Run run;

    SimpleMixer mixer;
    const size_t num_channels = 4;
    const size_t buffer_size = 128;
    const float master_gain = 0.5f;

    mixer.configure(num_channels, buffer_size, master_gain);

    // Setup buffers
    std::vector<std::vector<float>> input_data(num_channels, std::vector<float>(buffer_size));
    std::vector<const float*> input_ptrs(num_channels);
    for (size_t i = 0; i < num_channels; ++i) {
        input_ptrs[i] = input_data[i].data();
    }
    std::vector<float> output_data(buffer_size);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int iteration = 0; iteration < 500; ++iteration) {
        // Generate random input data
        for (size_t ch = 0; ch < num_channels; ++ch) {
            for (size_t i = 0; i < buffer_size; ++i) {
                input_data[ch][i] = dist(rng);
            }
        }

        // Compute expected output independently
        std::vector<float> expected(buffer_size);
        for (size_t i = 0; i < buffer_size; ++i) {
            float sum = 0.0f;
            for (size_t ch = 0; ch < num_channels; ++ch) {
                sum += input_data[ch][i];
            }
            expected[i] = sum * master_gain;
        }

        rtk::guarded_void(run, [&] {
            mixer.process(input_ptrs.data(), output_data.data());
        });

        for (size_t i = 0; i < buffer_size; ++i) {
            rtk::expect(run, rtk::near(output_data[i], expected[i]), "Sample match iter " + std::to_string(iteration));
        }
    }

    return rtk::report(run);
}
