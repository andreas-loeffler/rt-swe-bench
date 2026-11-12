#pragma once
#include <vector>
#include <cstdint>

class FirLowpassFilter {
public:
    /**
     * @brief Initializes the filter with coefficients.
     * Allowed to allocate memory.
     */
    void configure(std::vector<float> coefficients) {
        m_coeffs = std::move(coefficients);
        m_history.assign(m_coeffs.size(), 0.0f);
        m_write_idx = 0;
    }

    /**
     * @brief Processes a single sample through the FIR filter.
     * Real-time safe: no heap allocation, no locks, no exceptions.
     * 
     * @param sample The current input sample x[n].
     * @return The filtered output y[n].
     */
    float process(float sample) {
        const size_t N = m_coeffs.size();
        if (N == 0) return 0.0f;

        // Store current sample in the circular buffer
        m_history[m_write_idx] = sample;

        float output = 0.0f;
        for (size_t i = 0; i < N; ++i) {
            // The formula is y[n] = sum(h[i] * x[n-i])
            // m_history[m_write_idx] contains x[n]
            // We want to access x[n-i]
            int idx = static_cast<int>(m_write_idx) - static_cast<int>(i);
            
            // Wrap the index to stay within the bounds of the circular buffer
            while (idx < 0) {
                idx += static_cast<int>(N);
            }
            
            // Use modulo to ensure index is valid, though while loop handles negatives
            output += m_coeffs[i] * m_history[idx % static_cast<int>(N)];
        }

        // Advance the write pointer for the next sample
        m_write_idx = (m_write_idx + 1) % N;
        
        return output;
    }

private:
    std::vector<float> m_coeffs;
    std::vector<float> m_history;
    size_t m_write_idx = 0;
};
