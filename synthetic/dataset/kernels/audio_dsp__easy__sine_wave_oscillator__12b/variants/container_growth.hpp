#pragma once
#include <vector>
#include <cmath>

class SineWaveOscillator {
public:
    void configure(double sampleRate, double frequency) {
        m_sampleRate = sampleRate;
        m_frequency = frequency;
        m_phaseIncrement = (2.0 * M_PI * frequency) / sampleRate;
        m_phase = 0.0;
        
        // Clear the history buffer to start fresh
        m_history.clear();
    }

    double process() {
        double sample = std::sin(m_phase);
        m_phase += m_phaseIncrement;
        if (m_phase >= 2.0 * M_PI) {
            m_phase -= 2.0 * M_PI;
        }

        // Store sample in history for potential envelope analysis
        m_history.push_back(sample);

        return sample;
    }

private:
    double m_sampleRate = 44100.0;
    double m_frequency = 440.0;
    double m_phaseIncrement = 0.0;
    double m_phase = 0.0;
    std::vector<double> m_history;
};
