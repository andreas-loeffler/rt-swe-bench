#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    ZeroCrossingRate zcr;
    const size_t N = 4;
    zcr.configure(N);

    // Test Case 1: Basic crossing
    // Signal: 1.0, -1.0, 1.0, -1.0
    // Expectations:
    // push(1.0) -> available=1, ZCR=0.0
    // push(-1.0) -> available=2, crossings=1, ZCR=1/1 = 1.0
    // push(1.0) -> available=3, crossings=2, ZCR=2/2 = 1.0
    // push(-1.0) -> available=4, crossings=3, ZCR=3/3 = 1.0
    float vals[] = {1.0f, -1.0f, 1.0f, -1.0f};
    float expected[] = {0.0f, 1.0f, 1.0f, 1.0f};
    
    for (int i = 0; i < 4; ++i) {
        float res = rtk::guarded(run, [&] { return zcr.push(vals[i]); });
        rtk::expect(run, std::abs(res - expected[i]) < 1e-6f, "Basic crossing " + std::to_string(i));
    }

    // Test Case 2: Edge cases (Zeros)
    // The spec says "opposite signs". 0 is neither positive nor negative.
    // A transition 1.0 -> 0.0 is NOT a zero crossing.
    // A transition 0.0 -> -1.0 is NOT a zero crossing.
    // A transition 1.0 -> -1.0 IS a zero crossing.
    zcr.configure(N);
    
    // Sequence: 1.0, 0.0, -1.0, 0.0
    // push(1.0) -> 0.0
    // push(0.0) -> available=2, (1>0 && 0<0) || (1<0 && 0>0) = false, ZCR=0/1 = 0.0
    // push(-1.0) -> available=3, 
    //    pair(1,0)=no, pair(0,-1)=no, ZCR=0/2 = 0.0
    // push(0.0) -> available=4,
    //    pair(1,0)=no, pair(0,-1)=no, pair(-1,0)=no, ZCR=0/3 = 0.0
    float edge_vals[] = {1.0f, 0.0f, -1.0f, 0.0f};
    for (int i = 0; i < 4; ++i) {
        float res = rtk::guarded(run, [&] { return zcr.push(edge_vals[i]); });
        rtk::expect(run, std::abs(res - 0.0f) < 1e-6f, "Zero boundary check " + std::to_string(i));
    }

    // Test Case 3: Sliding window removal
    // Window size 3. Signal: [1.0, -1.0, 1.0] -> ZCR = 2/2 = 1.0
    // Next push: [ -1.0, 1.0, -1.0 ] -> ZCR = 2/2 = 1.0
    // Next push: [ 1.0, -1.0, 1.0 ] -> ZCR = 2/2 = 1.0
    zcr.configure(3);
    rtk::guarded(run, [&] { return zcr.push(1.0f); });
    rtk::guarded(run, [&] { return zcr.push(-1.0f); });
    float r1 = rtk::guarded(run, [&] { return zcr.push(1.0f); });
    rtk::expect(run, std::abs(r1 - 1.0f) < 1e-6f, "Window full ZCR");
    
    float r2 = rtk::guarded(run, [&] { return zcr.push(-1.0f); });
    rtk::expect(run, std::abs(r2 - 1.0f) < 1e-6f, "Window slide ZCR");

    return rtk::report(run);
}
