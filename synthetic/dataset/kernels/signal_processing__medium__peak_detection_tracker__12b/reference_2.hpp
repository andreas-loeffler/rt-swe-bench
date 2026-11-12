#pragma once
#include <cstdint>
#include <limits>

/**
 * PeakDetector identifies local maxima in a real-time signal stream.
 * 
 * A peak is confirmed at index i when:
 * 1. sample[i] > threshold
 * 2. sample[i] > sample[i-1]
 * 3. sample[i] > sample[i+1]
 * 4. index i >= last_peak_index + min_distance
 * 
 * Due to the look-ahead requirement (sample[i+1]), the result for index i
 * is returned when process() is called with sample[i+1].
 */
class PeakDetector {
public:
    void configure(float threshold, uint32_t min_distance) {
        m_threshold = threshold;
        m_min_distance = min_distance;
        
        // Reset state
        m_prev_sample = std::numeric_limits<float>::lowest();
        m_candidate_sample = std::numeric_limits<float>::lowest();
        m_has_candidate = false;
        m_samples_since_peak = m_min_distance; 
    }

    bool process(float sample) {
        bool detected = false;

        // Check if the previous sample was a peak.
        // We need: 
        // 1. A candidate existed (sample[i-1] > sample[i-2])
        // 2. The candidate was above threshold
        // 3. Current sample is smaller than candidate (sample[i] < sample[i-1])
        // 4. Distance constraint is met
        if (m_has_candidate && 
            m_candidate_sample > m_threshold && 
            m_candidate_sample > sample &&
            m_samples_since_peak >= m_min_distance) {
            
            detected = true;
            m_samples_since_peak = 0;
            // Reset candidate because this peak is now confirmed
            m_has_candidate = false;
        }

        // Update candidate tracking for the next call.
        // A sample at index i is a candidate if it's strictly greater than sample[i-1].
        if (sample > m_prev_sample) {
            m_candidate_sample = sample;
            m_has_candidate = true;
        } else {
            // If current sample <= previous, the previous was not a peak,
            // and the current cannot be a peak candidate unless it's > next.
            m_has_candidate = false;
        }

        m_prev_sample = sample;
        m_samples_since_peak++;

        return detected;
    }

private:
    float m_threshold = std::numeric_limits<float>::infinity();
    uint32_t m_min_distance = 0;

    float m_prev_sample = std::numeric_limits<float>::lowest();
    float m_candidate_sample = std::numeric_limits<float>::lowest();
    bool m_has_candidate = false;
    uint32_t m_samples_since_peak = 0;
};
