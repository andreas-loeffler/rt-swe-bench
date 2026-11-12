#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <array>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    // Test Case 1: Standard multi-step interpolation
    {
        TransformInterpolator interpolator;
        const size_t steps = 10;
        interpolator.configure(steps);

        float start_mat[16];
        float end_mat[16];
        for (int i = 0; i < 16; ++i) {
            start_mat[i] = static_cast<float>(i);
            end_mat[i] = static_cast<float>(i * 2);
        }

        for (size_t s = 0; s < steps; ++s) {
            float result[16] = {0}; // Initialize to detect missing indices
            rtk::guarded_void(run, [&]() {
                interpolator.interpolate(start_mat, end_mat, s, result);
            });

            float t = static_cast<float>(s) / static_cast<float>(steps - 1);
            for (int i = 0; i < 16; ++i) {
                float expected = start_mat[i] + (end_mat[i] - start_mat[i]) * t;
                rtk::expect(run, rtk::near(result[i], expected), "Step " + std::to_string(s) + " index " + std::to_string(i));
            }
        }
    }

    // Test Case 2: Single step (Boundary condition m_num_steps <= 1)
    // This catches: line 14: 0 -> 1 or < -> <= (if logic changes)
    {
        TransformInterpolator interpolator;
        interpolator.configure(1);
        float start_mat[16];
        float end_mat[16];
        for(int i=0; i<16; ++i) {
            start_mat[i] = 1.0f;
            end_mat[i] = 2.0f;
        }
        float result[16] = {0};
        
        rtk::guarded_void(run, [&]() {
            interpolator.interpolate(start_mat, end_mat, 0, result);
        });
        for (int i = 0; i < 16; ++i) {
            rtk::expect(run, rtk::near(result[i], 1.0f), "Single step (1) must return start");
        }
    }

    // Test Case 3: Zero steps (Boundary condition m_num_steps <= 1)
    {
        TransformInterpolator interpolator;
        interpolator.configure(0);
        float start_mat[16];
        float end_mat[16];
        for(int i=0; i<16; ++i) {
            start_mat[i] = 1.0f;
            end_mat[i] = 2.0f;
        }
        float result[16] = {0};
        
        rtk::guarded_void(run, [&]() {
            interpolator.interpolate(start_mat, end_mat, 0, result);
        });
        for (int i = 0; i < 16; ++i) {
            rtk::expect(run, rtk::near(result[i], 1.0f), "Zero steps must return start");
        }
    }

    // Test Case 4: Strict index checking for loop boundaries (0..15)
    // This catches: line 14: 16 -> 17 or 16 -> 15, and line 19: 16 -> 17 or 16 -> 15
    {
        TransformInterpolator interpolator;
        interpolator.configure(2);
        float start_mat[32] = {0}; // Larger buffer to detect OOB writes
        float end_mat[32] = {0};
        for(int i=0; i<16; ++i) {
            start_mat[i] = 1.0f;
            end_mat[i] = 2.0f;
        }

        float result[32] = {0};
        rtk::guarded_void(run, [&]() {
            interpolator.interpolate(start_mat, end_mat, 1, result);
        });
        
        // Verify index 0 and 15 are correct
        rtk::expect(run, rtk::near(result[0], 2.0f), "Index 0 must be updated");
        rtk::expect(run, rtk::near(result[15], 2.0f), "Index 15 must be updated");
        
        // Verify index 16 is NOT updated (catches loop going to 17)
        rtk::expect(run, rtk::near(result[16], 0.0f), "Index 16 must NOT be updated");
    }

    return rtk::report(run);
}
