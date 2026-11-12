// Allocating variant: formats with a std::ostringstream per call.
#pragma once
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

class StatusLine
{
public:
  void configure(std::size_t max_length) { line_.reserve(max_length); }

  std::string_view format(std::uint32_t id, std::string_view joint, double position, double velocity)
  {
    std::ostringstream os;
    os << std::fixed << std::setprecision(6) << "id=" << id << " joint=" << joint << " pos=" << position
       << " vel=" << velocity;
    line_ = os.str();
    return line_;
  }

private:
  std::string line_;
};
