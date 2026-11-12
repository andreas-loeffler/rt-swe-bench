// Allocating variant: the mean is computed over a copy of the window, which
// allocates on every call.
#pragma once
#include <cstddef>
#include <numeric>
#include <vector>

class MovingAverage
{
public:
  void configure(std::size_t window)
  {
    window_ = window;
    buf_.assign(window, 0.0);
    head_ = 0;
    count_ = 0;
  }

  double process(double x)
  {
    buf_[head_] = x;
    head_ = (head_ + 1) % window_;
    if (count_ < window_) {++count_;}
    std::vector<double> recent(buf_.begin(), buf_.begin() + static_cast<long>(count_));
    if (count_ == window_) {recent.assign(buf_.begin(), buf_.end());}
    return std::accumulate(recent.begin(), recent.end(), 0.0) / static_cast<double>(recent.size());
  }

private:
  std::vector<double> buf_;
  std::size_t window_ = 1, head_ = 0, count_ = 0;
};
