#pragma once

class PeakHoldAnalyzer {
public:
    struct AnalysisResult {
        float max_peak;
        float min_peak;
    };

    void configure(float decay_time, float min_sat, float max_sat) {
        // TODO: implement
    }

    void process(float sample, float dt) {
        // TODO: implement
    }

    AnalysisResult get_peaks() const {
        // TODO: implement
        return {0.0f, 0.0f};
    }
};
