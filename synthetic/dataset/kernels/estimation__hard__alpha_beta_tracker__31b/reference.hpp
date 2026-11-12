#pragma once

#include <cmath>

template <typename T>
class StateEstimator {
public:
    struct Gains {
        T alpha;
        T beta;
    };

    void update(T measurement, T dt, Gains g) {
        T prediction = pos + vel * dt;
        T error = measurement - prediction;
        pos = prediction + g.alpha * error;
        vel = vel + (g.beta * error) / dt;
    }

    void initialize(T val) {
        pos = val;
        vel = 0;
    }

    void clear() {
        pos = 0;
        vel = 0;
    }

    T get_pos() const { return pos; }
    T get_vel() const { return vel; }

private:
    T pos = 0;
    T vel = 0;
};

class AlphaBetaTracker {
public:
    void configure(double dt, double alpha_fast, double beta_fast, double alpha_slow, double beta_slow, double threshold) {
        this->dt_ = dt;
        fast_gains_ = {alpha_fast, beta_fast};
        slow_gains_ = {alpha_slow, beta_slow};
        threshold_ = threshold;
        initialized_ = false;
    }

    void push(double measurement) {
        if (!initialized_) {
            estimator_.initialize(measurement);
            initialized_ = true;
            return;
        }

        double prediction = estimator_.get_pos() + estimator_.get_vel() * dt_;
        double error = measurement - prediction;

        if (std::abs(error) > threshold_) {
            estimator_.update(measurement, dt_, fast_gains_);
        } else {
            estimator_.update(measurement, dt_, slow_gains_);
        }
    }

    double get_position() const {
        return estimator_.get_pos();
    }

    double get_velocity() const {
        return estimator_.get_vel();
    }

    void reset() {
        estimator_.clear();
        initialized_ = false;
    }

private:
    StateEstimator<double> estimator_;
    double dt_ = 0.0;
    StateEstimator<double>::Gains fast_gains_ = {0, 0};
    StateEstimator<double>::Gains slow_gains_ = {0, 0};
    double threshold_ = 0.0;
    bool initialized_ = false;
};
