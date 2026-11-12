#pragma once
#include <vector>

/**
 * FirFilter implements a discrete-time Finite Impulse Response filter.
 * It uses a circular buffer to ensure that the process() method remains
 * real-time safe by avoiding memory shifts.
 */
class FirFilter {
public:
    void configure(const std::vector<float>& coefficients) {
        // Memory allocation is permitted here as per requirements.
        m_coeffs = coefficients;
        m_delayLine.assign(m_coeffs.size(), 0.0f);
        m_writeIdx = 0;
    }

    float process(float input) {
        // Write the current input into the circular buffer
        m_delayLine[m_writeIdx] = input;

        float output = 0.0f;
        size_t numTaps = m_coeffs.size();
        size_t readIdx = m_writeIdx;

        // Compute the convolution sum: y[n] = sum_{i=0}^{M-1} x[n-i] * h[i]
        for (size_t i = 0; i < numTaps; ++i) {
            output += m_delayLine[readIdx] * m_coeffs[i];
            
            // Move backward through the circular buffer (x[n], x[n-1], ...)
            if (readIdx == 0) {
                readIdx = numTaps - 1;
            } else {
                --readIdx;
            }
        }

        // Advance write pointer for the next sample
        if (++m_writeIdx >= numTaps) {
            m_writeIdx = 0;
        }

        return output;
    }

private:
    std::vector<float> m_coeffs;
    std::vector<float> m_delayLine;
    size_t m_writeIdx = 0;
};
