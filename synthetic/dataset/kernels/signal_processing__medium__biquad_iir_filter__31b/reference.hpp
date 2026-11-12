#pragma once
#include <cmath>

class BiquadFilter {
public:
    void configure(float b0, float b1, float b2, float a1, float a2) {
        this->b0 = b0;
        this->b1 = b1;
        this->b2 = b2;
        this->a1 = a1;
        this->a2 = a2;
        reset();
    }

    float process(float input) noexcept {
        // Direct Form II
        // w[n] = x[n] - a1*w[n-1] - a2*w[n-2]
        float w_n = input - (a1 * z1) - (a2 * z2);
        
        // y[n] = b0*w[n] + b1*w[n-1] + b2*w[n-2]
        float y_n = (b0 * w_n) + (b1 * z1) + (b2 * z2);
        
        // Update state
        z2 = z1;
        z1 = w_n;
        
        return y_n;
    }

    void reset() noexcept {
        z1 = 0.0f;
        z2 = 0.0f;
    }

private:
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f; // w[n-1]
    float z2 = 0.0f; // w[n-2]
};
