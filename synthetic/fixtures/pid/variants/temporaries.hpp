// Allocating variant: computes errors into a temporary vector per call.
#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

class Pid
{
public:
  void configure(std::size_t dofs, double kp, double ki, double kd, double dt, double i_limit)
  {
    dofs_ = dofs; kp_ = kp; ki_ = ki; kd_ = kd; dt_ = dt; i_limit_ = i_limit;
    integral_.assign(dofs, 0.0);
    prev_error_.assign(dofs, 0.0);
    first_ = true;
  }

  void process(const double * reference, const double * measured, double * command)
  {
    std::vector<double> error(reference, reference + dofs_);
    for (std::size_t i = 0; i < dofs_; ++i) {error[i] -= measured[i];}
    for (std::size_t i = 0; i < dofs_; ++i)
    {
      integral_[i] = std::clamp(integral_[i] + ki_ * error[i] * dt_, -i_limit_, i_limit_);
      const double d = first_ ? 0.0 : (error[i] - prev_error_[i]) / dt_;
      command[i] = kp_ * error[i] + integral_[i] + kd_ * d;
    }
    prev_error_ = error;
    first_ = false;
  }

private:
  std::size_t dofs_ = 0;
  double kp_ = 0, ki_ = 0, kd_ = 0, dt_ = 1, i_limit_ = 0;
  std::vector<double> integral_, prev_error_;
  bool first_ = true;
};
