#pragma once

#include <stdexcept>

/**
 * LeadLagCompensator implements a first-order discrete-time filter 
 * for lead-lag compensation.
 * 
 * Difference Equation:
 * y[n] = alpha * x[n] + (1 - alpha - beta) * y[n-1] + beta * x[n-1]
 */
class LeadLagCompensator {
public:
    void configure(double alpha, double beta) {
        if (alpha == 0.0 || beta == 0.0) {
            throw std::invalid_argument("Alpha and beta must be non-zero.");
        }

        m_alpha = alpha;
        m_beta = beta;
        m_feedback_coeff = 1.0 - alpha - beta;
        
        reset();
    }

    double process(double input) {
        // The real-time path: no allocation, no locks, no throws.
        // y[n] = alpha*x[n] + (1-alpha-beta)*y[n-1] + beta*x[n-1]
        double output = (m_alpha * input) + 
                        (m_feedback_coeff * m_last_output) + 
                        (m_beta * m_last_input);

        m_last_input = input;
        m_last_output = output;

        return output;
    }

    void reset() {
        m_last_input = 0.0;
        m_last_output = 0.0;
    }

private:
    double m_alpha = 0.0;
    double m_beta = 0.0;
    double m_feedback_coeff = 0.0;
    
    double m_last_input = 0.0;
    double m_last_output = 0.0;
};
