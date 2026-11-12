#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <random>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t num_targets = 600;
    DistanceCalculator k;
    k.configure(num_targets);

    // Pre-generate targets and reference points
    std::vector<float> targets(num_targets * 3);
    std::vector<float> ref_points[1000]; // 1000 different reference point sets
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

    for (auto& val : targets) val = dist(gen);
    for (int i = 0; i < 1000; ++i) {
        ref_points[i].resize(3);
        for (int j = 0; j < 3; ++j) ref_points[i][j] = dist(gen);
    }

    // Test batch distance calculations
    for (int i = 0; i < 500; ++i) {
        // Update targets occasionally to exercise set_targets
        if (i % 100 == 0) {
            rtk::guarded_void(run, [&] {
                k.set_targets(targets.data());
            });
        }

        float results[num_targets];
        const float* current_ref = ref_points[i].data();

        rtk::guarded_void(run, [&] {
            k.process(current_ref, results);
        });

        // Independent verification
        for (size_t j = 0; j < num_targets; ++j) {
            float dx = targets[j * 3 + 0] - current_ref[0];
            float dy = targets[j * 3 + 1] - current_ref[1];
            float dz = targets[j * 3 + 2] - current_ref[2];
            float expected = std::sqrt(dx * dx + dy * dy + dz * dz);
            rtk::expect(run, rtk::near(results[j], expected), "distance_match_" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    return rtk::report(run);
}
