#pragma once

#include <algorithm>

class PeakHoldAnalyzer {
public:
    struct AnalysisResult {
        float max_peak;
        float min_peak;
    };

    /**
     * @brief Configures the analyzer parameters.
     * Allocations are allowed here as it's called before the real-time loop.
     */
    void configure(float decay_time, float min_sat, float max_sat) {
        decay_time_ = decay_time;
        min_sat_ = min_sat;
        max_sat_ = max_sat;

        // Initialize peaks to the boundaries to ensure they start at valid states
        // and set the current signal reference to the midpoint.
        max_peak_ = max_sat;
        min_peak_ = min_sat;
    }

    /**
     * @brief Processes a single sample in real-time.
     * No heap allocation, locking, or I/O allowed.
     */
    void process(float sample, float dt) {
        // 1. Saturation
        float clamped = std::clamp(sample, min_sat_, max_sat_);

        // 2. Peak Tracking (Updates and Timer Resets)
        // If current sample exceeds max_peak, it becomes the new max_peak.
        if (clamped > max_peak_) {
            max_peak_ = clamped;
        } 
        // If current sample is below min_peak, it becomes the new min_peak.
        else if (clamped < min_peak_) {
            min_peak_ = clamped;
        }

        // 3. Linear Decay
        // The decay is: peak_new = peak_old - (peak_old - clamped) * (dt / decay_time)
        // We use a simple linear interpolation (lerp) logic.
        // If dt is large enough to fully decay, we cap it at the clamped value.
        
        float max_decay_ratio = std::min(1.0f, dt / decay_time_);
        if (max_decay_ratio > 0.0f) {
            max_peak_ -= (max_peak_ - clamped) * max_decay_ratio;
        }

        float min_decay_ratio = std::min(1.0f, dt / decay_time_);
        if (min_decay_ratio > 0.0f) {
            min_peak_ += (clamped - min_peak_) * min_decay_ratio;
        }
    }

    /**
     * @brief Returns the current peaks in real-time.
     */
    AnalysisResult get_peaks() const {
        return {max_peak_, min_peak_};
    }

private:
    float decay_time_ = 1.0f;
    float min_sat_ = -1e6f;
    float max_sat_ = 1e6f;
    
    float max_peak_ = 0.0f;
    float min_peak_ = 0.0f;
};
