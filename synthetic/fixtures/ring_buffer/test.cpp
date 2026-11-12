// Tier 1 test: ring_buffer. 3,000 real-time operations (two pushes per pop, so
// the buffer also runs full) against a reference model built outside the guard.
#include <deque>
#include <string>

#include "kernel.hpp"
#include "rtk/check.hpp"

int main()
{
  rtk::setup();
  const std::size_t capacity = 100;
  RingBuffer rb;
  rb.configure(capacity);
  std::deque<double> model;
  rtk::Run run;
  for (int i = 0; i < 3000; ++i)
  {
    if (i % 3 != 2)
    {
      const double x = 0.5 * i;
      const bool ok = rtk::guarded(run, [&] { return rb.push(x); });
      const bool expected = model.size() < capacity;
      if (expected) {model.push_back(x);}
      rtk::expect(run, ok == expected, "push " + std::to_string(i));
    }
    else
    {
      double out = -1.0;
      const bool ok = rtk::guarded(run, [&] { return rb.pop(out); });
      const bool expected = !model.empty();
      rtk::expect(run, ok == expected && (!ok || out == model.front()), "pop " + std::to_string(i));
      if (expected) {model.pop_front();}
    }
    rtk::expect(run, rb.size() == model.size(), "size after " + std::to_string(i));
  }
  return rtk::report(run);
}
