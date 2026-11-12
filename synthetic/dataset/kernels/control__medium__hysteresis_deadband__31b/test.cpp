#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    HysteresisDeadband k;
    const double act = 1.0;
    const double rel = 0.5;
    k.configure(act, rel);

    // Test suite: sequence of inputs and expected results
    struct TestCase {
        double input;
        double expected;
    };

    std::vector<TestCase> cases;
    
    // 1. Start at zero, stay zero (below activate)
    for(int i=0; i<100; ++i) cases.push_back({0.9, 0.0});
    
    // 2. Trigger active (positive)
    cases.push_back({1.1, 1.1});
    
    // 3. Stay active (above release)
    for(int i=0; i<100; ++i) cases.push_back({0.6, 0.6});
    
    // 4. Fall back to zero (below release)
    cases.push_back({0.4, 0.0});
    
    // 5. Stay zero (between release and activate)
    for(int i=0; i<100; ++i) cases.push_back({0.8, 0.0});
    
    // 6. Trigger active (negative)
    cases.push_back({-1.1, -1.1});
    
    // 7. Stay active (negative, above release)
    for(int i=0; i<100; ++i) cases.push_back({-0.6, -0.6});
    
    // 8. Fall back to zero (negative, below release)
    cases.push_back({-0.4, 0.0});
    
    // 9. Edge case: exactly at activate
    cases.push_back({1.0, 1.0});
    
    // 10. Edge case: exactly at release
    cases.push_back({0.5, 0.0});

    // Execute real-time calls
    for (const auto& tc : cases) {
        double result = rtk::guarded(run, [&]() { 
            return k.process(tc.input); 
        });
        rtk::near(result, tc.expected, 1e-9);
        rtk::expect(run, std::abs(result - tc.expected) < 1e-9, "Value match");
    }

    return rtk::report(run);
}
