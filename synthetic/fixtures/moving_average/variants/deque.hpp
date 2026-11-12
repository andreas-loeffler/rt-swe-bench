// Allocating variant: the window lives in a std::deque, which allocates a new
// block every few hundred bytes of push_back as the front blocks are freed.
#pragma once
#include <cstddef>
#include <deque>

class MovingAverage
{
public:
  void configure(std::size_t window)
  {
    window_ = window;
    buf_.clear();
    sum_ = 0.0;
  }

  double process(double x)
  {
    buf_.push_back(x);
    sum_ += x;
    if (buf_.size() > window_)
    {
      sum_ -= buf_.front();
      buf_.pop_front();
    }
    return sum_ / static_cast<double>(buf_.size());
  }

private:
  std::deque<double> buf_;
  std::size_t window_ = 1;
  double sum_ = 0.0;
};
