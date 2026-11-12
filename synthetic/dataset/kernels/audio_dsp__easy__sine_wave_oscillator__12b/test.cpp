#include "kernel.hpp"
#include "rtk/check.hpp"
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    const double sampleRate = 44100.0;
    const double frequency = 440.0;
    const double pi = 3.14159265358979323846;
    const double twoPi = 2.0 * pi;

    SineWaveOscillator osc;
    osc.configure(sampleRate, frequency);

    double phaseIncrement = (twoPi * frequency) / sampleRate;
    double currentPhase = 0.0;

    for (int i = 0; i < 1000; ++i) {
        double expected = std::sin(currentPhase);
        
        double result = rtk::guarded(run, [&] { 
            return osc.process(); 
        });

        rtk::expect(run, result == expected, "Oscillator sample match");

        currentPhase += phaseIncrement;
        if (currentPhase >= twoPi) {
            // Simulate the wrap-around logic in the kernel
            currentPhase -= (int)(currentPhase / twoPi) * twoPi;
        }
    }

    return rtk::report(run);
}
