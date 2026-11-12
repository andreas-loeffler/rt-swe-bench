#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    PlanarArmKinematics k;
    const size_t num_joints = 3;
    k.configure(num_joints);

    // Test various joint configurations
    // We test 500 different configurations to ensure coverage.
    for (int i = 0; i < 500; ++i) {
        // Generate deterministic inputs
        std::vector<double> angles(num_joints);
        double sum_x = 0.0;
        double sum_y = 0.0;
        double current_angle = 0.0;

        for (size_t j = 0; j < num_joints; ++j) {
            // Mix of small, large, and wrap-around values
            angles[j] = (static_cast<double>(i) * 0.01337) + (j * 0.5);
            current_angle += angles[j];
            sum_x += std::cos(current_angle);
            sum_y += std::sin(current_angle);
        }

        auto result = rtk::guarded(run, [&] { 
            return k.process(angles); 
        });

        rtk::expect(run, rtk::near(result.first, sum_x, 1e-9), "X coordinate match");
        rtk::expect(run, rtk::near(result.second, sum_y, 1e-9), "Y coordinate match");
    }

    // Edge cases
    std::vector<double> zeros(num_joints, 0.0);
    auto res_zeros = rtk::guarded(run, [&] { return k.process(zeros); });
    rtk::expect(run, rtk::near(res_zeros.first, 3.0, 1e-9), "Zero angles X");
    rtk::expect(run, rtk::near(res_zeros.second, 0.0, 1e-9), "Zero angles Y");

    return rtk::report(run);
}
