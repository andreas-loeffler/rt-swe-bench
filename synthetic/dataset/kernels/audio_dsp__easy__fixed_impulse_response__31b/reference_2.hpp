#pragma once

#include <span>
#include <vector>

class ImpulseResponseConvolver {
public:
    void configure(std::span<const float> coefficients) {
        m_coefficients.assign(coefficients.begin(), coefficients.end());
        m_history.assign(m_coefficients.size(), 0.0f);
    }

    float process(float input) {
        // Update history: shift right and insert new sample at the head
        // Optimization: We treat the history as a circular buffer or shift.
        // Given 'easy' constraints and the nature of FIR, we update the history:
        for (size_t i = m_history.size() - 1; i > 0; --i) {
            m_history[i] = m_history[i - 1];
        }
        m_history[0] = input;

        float output = 0.0f;
        // Perform Sum of Products (Dot Product)
        for (size_t i = 0; i < m_history.size(); ++i) {
            output += m_history[i] * m_coefficients[i];
        }

        return output;
    }

private:
    std::vector<float> m_coefficients;
    std::vector<float> m_history;
};
