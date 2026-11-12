#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <random>

int main() {
    rtk::setup();
    rtk::Run run;

    PointCloudTransformer k;
    const size_t MAX_POINTS = 100;
    k.configure(MAX_POINTS);

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

    std::vector<float> inputs(MAX_POINTS * 3);
    std::vector<float> outputs(MAX_POINTS * 3);
    std::vector<float> matrix(16);

    for (int i = 0; i < 600; ++i) {
        // Setup deterministic random inputs
        for (auto& val : inputs) val = dist(gen);
        for (auto& val : matrix) val = dist(gen);
        
        // Ensure matrix is somewhat sane for the test, though it works for any
        size_t current_batch = (i % (MAX_POINTS + 1));

        rtk::guarded_void(run, [&] {
            k.transform(inputs.data(), outputs.data(), matrix.data(), current_batch);
        });

        // Independent validation
        for (size_t j = 0; j < current_batch; ++j) {
            float x = inputs[j * 3 + 0];
            float y = inputs[j * 3 + 1];
            float z = inputs[j * 3 + 2];
            
            float expected_x = matrix[0] * x + matrix[1] * y + matrix[2] * z + matrix[3];
            float expected_y = matrix[4] * x + matrix[5] * y + matrix[6] * z + matrix[7];
            float expected_z = matrix[8] * x + matrix[9] * y + matrix[10] * z + matrix[11];

            rtk::expect(run, rtk::near(outputs[j * 3 + 0], expected_x), "X transform");
            rtk::expect(run, rtk::near(outputs[j * 3 + 1], expected_y), "Y transform");
            rtk::expect(run, rtk::near(outputs[j * 3 + 2], expected_z), "Z transform");
        }
    }

    return rtk::report(run);
}
