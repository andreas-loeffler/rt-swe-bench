#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    PlanarForwardKinematics k;
    const size_t MAX_PTS = 1000;
    const double L1 = 1.0;
    const double L2 = 0.5;
    
    k.configure(MAX_PTS, L1, L2);

    // Test Case 1: Normal trajectory (500 points)
    const size_t N1 = 500;
    std::vector<double> inputs1(N1 * 2);
    std::vector<PlanarForwardKinematics::Point> expected1(N1);
    
    for (size_t i = 0; i < N1; ++i) {
        double q1 = i * 0.01;
        double q2 = i * 0.02;
        inputs1[2 * i] = q1;
        inputs1[2 * i + 1] = q2;
        
        expected1[i].x = L1 * std::cos(q1) + L2 * std::cos(q1 + q2);
        expected1[i].y = L1 * std::sin(q1) + L2 * std::sin(q1 + q2);
    }

    size_t processed1 = rtk::guarded(run, [&] { 
        return k.process(inputs1.data(), N1); 
    });

    rtk::expect(run, processed1 == N1, "Process count N1");
    for (size_t i = 0; i < N1; ++i) {
        auto res = k.get_result(i);
        rtk::near(run, res.x, expected1[i].x);
        rtk::near(run, res.y, expected1[i].y);
    }

    // Test Case 2: Overflow (Request more than max_trajectory_length)
    const size_t N2 = MAX_PTS + 100;
    std::vector<double> inputs2(N2 * 2, 0.0);
    
    size_t processed2 = rtk::guarded(run, [&] { 
        return k.process(inputs2.data(), N2); 
    });

    rtk::expect(run, processed2 == MAX_PTS, "Process count capped at MAX_PTS");

    // Test Case 3: Zero points
    size_t processed3 = rtk::guarded(run, [&] { 
        return k.process(inputs1.data(), 0); 
    });
    rtk::expect(run, processed3 == 0, "Process count 0");

    // Test Case 4: Null pointer
    size_t processed4 = rtk::guarded(run, [&] { 
        return k.process(nullptr, 10); 
    });
    rtk::expect(run, processed4 == 0, "Process null pointer");

    return rtk::report(run);
}
