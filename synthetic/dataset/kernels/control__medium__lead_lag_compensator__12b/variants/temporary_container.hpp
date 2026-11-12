#pragma once
#include <vector>
#include <cmath>

class LeadLagCompensator {
public:
    void configure(double alpha, double beta) {
        a_ = alpha;
        b_ = beta;
        prev_x_ = 0.0;
        prev_y_ = 0.0;
    }

    double process(double x) {
        std::vector<double> coefficients = {a_, (1.0 - a_ - b_), b_};
        
        double y = coefficients[0] * x + coefficients[1] * prev_y_ + coefficients[2] * prev_x_;
        
        prev_x_ = x;
        prev_y_ = y;
        return y;
    }

    void reset() {
        prev_x_ = 0.0;
        prev_y_ = 0.0;
    }

private:
    double a_ = 0.0;
    double b_ = 0.0;
    double prev_x_ = 0.0;
    double prev_y_ = 0.0;
};
