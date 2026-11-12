#pragma once

/**
 * AlphaBetaFilter
 * 
 * Implements a first-order recursive filter (also known as an Exponential Moving Average).
 * The update equation is: state = alpha * measurement + (1 - alpha) * state.
 * 
 * This implementation satisfies real-time constraints: the process method is O(1),
 * performs no heap allocations, no locking, and no I/O.
 */
class AlphaBetaFilter {
public:
    /**
     * Prepares the filter for real-time execution.
     * 
     * @param initial_alpha The starting smoothing factor \alpha \in [0, 1].
     * @param initial_value The starting state of the filter.
     */
    void configure(double initial_alpha, double initial_value) {
        m_alpha = initial_alpha;
        m_state = initial_value;
    }

    /**
     * Processes a new measurement and returns the updated smoothed estimate.
     * 
     * @param measurement The new raw data point.
     * @param alpha The smoothing factor to use for this specific step \in [0, 1].
     * @return The updated smoothed state.
     */
    double process(double measurement, double alpha) {
        // Formula: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        // Rewritten as: y[n] = y[n-1] + alpha * (x[n] - y[n-1]) for better numerical stability
        // and slightly fewer operations.
        m_state = m_state + alpha * (measurement - m_state);
        m_alpha = alpha; // Update internal alpha for state tracking if needed
        return m_state;
    }

private:
    double m_state = 0.0;
    double m_alpha = 0.0;
};
