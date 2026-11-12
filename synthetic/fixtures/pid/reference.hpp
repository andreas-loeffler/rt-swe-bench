// Reference: per-DOF state sized in configure().
#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

class Pid
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t dofs, double kp, double ki, double kd, double dt, double i_limit)
  {
    dofs_ = dofs; kp_ = kp; ki_ = ki; kd_ = kd; dt_ = dt; i_limit_ = i_limit;
    integral_.assign(dofs, 0.0);
    prev_error_.assign(dofs, 0.0);
    first_ = true;
  }

  // Real-time: one control step for all DOFs.
  void process(const double * reference, const double * measured, double * command)
  {
    for (std::size_t i = 0; i < dofs_; ++i)
    {
      const double e = reference[i] - measured[i];
      integral_[i] = std::clamp(integral_[i] + ki_ * e * dt_, -i_limit_, i_limit_);
      const double d = first_ ? 0.0 : (e - prev_error_[i]) / dt_;
      command[i] = kp_ * e + integral_[i] + kd_ * d;
      prev_error_[i] = e;
    }
    first_ = false;
  }

private:
  std::size_t dofs_ = 0;
  double kp_ = 0, ki_ = 0, kd_ = 0, dt_ = 1, i_limit_ = 0;
  std::vector<double> integral_, prev_error_;
  bool first_ = true;
};
