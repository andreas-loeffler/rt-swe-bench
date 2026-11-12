#pragma once
#include <vector>

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
        auto state = get_current_state();
        
        float w_n = input - (a1 * state[0]) - (a2 * state[1]);
        float y_n = (b0 * w_n) + (b1 * state[0]) + (b2 * state[1]);
        
        z1 = w_n;
        z2 = state[0];
        
        return y_n;
    }

    void reset() noexcept {
        z1 = 0.0f;
        z2 = 0.0f;
    }

private:
    std::vector<float> get_current_state() {
        return {z1, z2};
    }

    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f;
    float z2 = 0.0f;
};
