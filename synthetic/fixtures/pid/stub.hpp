#pragma once
#include <cstddef>

class Pid
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t dofs, double kp, double ki, double kd, double dt, double i_limit)
  {
    (void)dofs; (void)kp; (void)ki; (void)kd; (void)dt; (void)i_limit;
  }

  // Real-time: one control step for all DOFs.
  void process(const double * reference, const double * measured, double * command)
  {
    (void)reference; (void)measured; (void)command;  // TODO
  }
};
