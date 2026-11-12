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
        std::vector<float> coefficients = {b0, b1, b2, a1, a2};
        std::vector<float> state = {z1, z2};

        float w_n = input - (coefficients[3] * state[0]) - (coefficients[4] * state[1]);
        float y_n = (coefficients[0] * w_n) + (coefficients[1] * state[0]) + (coefficients[2] * state[1]);

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
    float z1 = 0.0f;
    float z2 = 0.0f;
};
