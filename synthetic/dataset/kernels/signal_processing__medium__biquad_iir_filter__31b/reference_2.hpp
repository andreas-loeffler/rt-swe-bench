#pragma once

class BiquadFilter {
public:
    /**
     * Configures the filter coefficients and initializes state.
     * Called outside the real-time thread.
     */
    void configure(float b0, float b1, float b2, float a1, float a2) {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
        a1_ = a1;
        a2_ = a2;
        reset();
    }

    /**
     * Processes a single sample through the Direct Form II biquad filter.
     * Executed on a hard real-time path.
     * 
     * Difference Equation:
     * w[n] = x[n] - a1 * w[n-1] - a2 * w[n-2]
     * y[n] = b0 * w[n] + b1 * w[n-1] + b2 * w[n-2]
     */
    float process(float input) {
        // Calculate intermediate state w[n]
        const float w_n = input - (a1_ * w1_) - (a2_ * w2_);

        // Calculate output y[n]
        const float y_n = (b0_ * w_n) + (b1_ * w1_) + (b2_ * w2_);

        // Update state registers for next iteration
        w2_ = w1_;
        w1_ = w_n;

        return y_n;
    }

    /**
     * Resets the internal delay registers to zero.
     * Executed on a hard real-time path.
     */
    void reset() {
        w1_ = 0.0f;
        w2_ = 0.0f;
    }

private:
    // Coefficients
    float b0_ = 0.0f;
    float b1_ = 0.0f;
    float b2_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;

    // Delay registers (w[n-1], w[n-2])
    float w1_ = 0.0f;
    float w2_ = 0.0f;
};
