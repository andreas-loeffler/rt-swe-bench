// Reference: ring buffer sized in configure(), running sum.
#pragma once
#include <cstddef>
#include <vector>

class MovingAverage
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t window)
  {
    window_ = window;
    buf_.assign(window, 0.0);
    head_ = 0;
    count_ = 0;
    sum_ = 0.0;
  }

  // Real-time: mean of the last `window` samples (fewer before the window is full).
  double process(double x)
  {
    if (count_ == window_) {sum_ -= buf_[head_];} else {++count_;}
    buf_[head_] = x;
    sum_ += x;
    head_ = (head_ + 1) % window_;
    return sum_ / static_cast<double>(count_);
  }

private:
  std::vector<double> buf_;
  std::size_t window_ = 1, head_ = 0, count_ = 0;
  double sum_ = 0.0;
};
