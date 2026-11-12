#pragma once

class QuinticSplineGenerator {
public:
    struct State {
        double position;
        double velocity;
        double acceleration;
    };

    void configure(double duration) {
        // TODO: implement
    }

    void set_targets(double start_pos, double end_pos) {
        // TODO: implement
    }

    State step(double t) {
        // TODO: implement
        return {0.0, 0.0, 0.0};
    }
};
