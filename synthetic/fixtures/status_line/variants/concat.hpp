// Allocating variant: builds the line by std::string concatenation.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

class StatusLine
{
public:
  void configure(std::size_t max_length) { line_.reserve(max_length); }

  std::string_view format(std::uint32_t id, std::string_view joint, double position, double velocity)
  {
    line_ = "id=" + std::to_string(id) + " joint=" + std::string(joint) + " pos=" +
            std::to_string(position) + " vel=" + std::to_string(velocity);
    return line_;
  }

private:
  std::string line_;
};
