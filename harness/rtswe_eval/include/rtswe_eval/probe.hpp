// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Census probe: runs K guarded update() cycles on an already activated
// controller and aggregates every allocation by call site. Prints one
// human-readable block and one JSON line ("[rtswe-json] {...}") per probe.

#ifndef RTSWE_EVAL__PROBE_HPP_
#define RTSWE_EVAL__PROBE_HPP_

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "controller_interface/controller_interface_base.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/time.hpp"
#include "rtswe_eval/guarded_cycle.hpp"

namespace rtswe_eval
{

struct Site
{
  std::size_t allocs = 0;
  std::size_t bytes = 0;
  std::size_t cycles = 0;  // cycles in which this site allocated at least once
  int first_cycle = -1;    // index of the first and last guarded cycle with an allocation here
  int last_cycle = -1;
  std::string trace;       // symbolized backtrace of the first occurrence
};

struct ProbeResult
{
  std::string probe;
  std::string config;
  int cycles = 0;
  int allocating_cycles = 0;
  int failed_cycles = 0;  // update() returned something other than OK
  std::size_t allocs = 0;
  std::size_t bytes = 0;
  std::size_t first_cycle_bytes = 0;
  std::size_t steady_bytes_per_cycle = 0;  // bytes in the last cycle
  std::map<std::string, Site> sites;
};

inline std::string json_escape(const std::string & s)
{
  std::string o;
  for (char c : s)
  {
    switch (c)
    {
      case '"': o += "\\\""; break;
      case '\\': o += "\\\\"; break;
      case '\n': o += "\\n"; break;
      default: o += c;
    }
  }
  return o;
}

// `update` is called as update(time, period) inside the guard. Time stamps are
// built before the loop so constructing them is not counted. `before(k, time)`,
// if given, runs before cycle k outside the guard (e.g. a subscriber storing a
// fresh command).
using BeforeCycle = std::function<void(int, const rclcpp::Time &)>;

template <typename Update>
ProbeResult run_probe(
  const std::string & probe, const std::string & config, int cycles, Update && update,
  double period_s = 0.001, rclcpp::Time start = rclcpp::Time(0, 0, RCL_ROS_TIME),
  const BeforeCycle & before = {})
{
  ProbeResult res;
  res.probe = probe;
  res.config = config;
  res.cycles = cycles;
  const auto period = rclcpp::Duration::from_seconds(period_s);
  std::vector<rclcpp::Time> times;
  times.reserve(static_cast<std::size_t>(cycles));
  for (int k = 0; k < cycles; ++k) {times.push_back(start + period * k);}

  for (int k = 0; k < cycles; ++k)
  {
    if (before) {before(k, times[k]);}
    CycleReport report;
    const auto ret = guarded([&] {return update(times[k], period);}, report);
    if (ret != controller_interface::return_type::OK) {++res.failed_cycles;}
    if (k == 0) {res.first_cycle_bytes = report.bytes;}
    if (k == cycles - 1) {res.steady_bytes_per_cycle = report.bytes;}
    if (report.violations == 0) {continue;}
    ++res.allocating_cycles;
    res.allocs += report.violations;
    res.bytes += report.bytes;
    std::map<std::string, bool> seen;
    for (const auto & a : report.allocs)
    {
      const std::string key = site_of(a.trace);
      Site & s = res.sites[key];
      ++s.allocs;
      s.bytes += a.bytes;
      if (!seen[key]) {++s.cycles; seen[key] = true;}
      if (s.first_cycle < 0) {s.first_cycle = k;}
      s.last_cycle = k;
      if (s.trace.empty()) {s.trace = symbolize(a.trace);}
    }
  }
  return res;
}

inline void print(const ProbeResult & r)
{
  std::cout << "[rtswe] probe=" << r.probe << " (" << r.config << ")\n"
            << "[rtswe] cycles=" << r.cycles << " allocating_cycles=" << r.allocating_cycles
            << " allocs=" << r.allocs << " bytes=" << r.bytes
            << " first_cycle_bytes=" << r.first_cycle_bytes
            << " steady_bytes_per_cycle=" << r.steady_bytes_per_cycle
            << " failed_cycles=" << r.failed_cycles << "\n";
  for (const auto & [key, s] : r.sites)
  {
    std::cout << "[rtswe] site: " << key << "  allocs=" << s.allocs << " bytes=" << s.bytes
              << " cycles=" << s.cycles << "\n"
              << s.trace;
  }
  std::ostringstream j;
  j << "{\"probe\":\"" << json_escape(r.probe) << "\",\"config\":\"" << json_escape(r.config)
    << "\",\"cycles\":" << r.cycles << ",\"allocating_cycles\":" << r.allocating_cycles
    << ",\"allocs\":" << r.allocs << ",\"bytes\":" << r.bytes
    << ",\"first_cycle_bytes\":" << r.first_cycle_bytes
    << ",\"steady_bytes_per_cycle\":" << r.steady_bytes_per_cycle
    << ",\"failed_cycles\":" << r.failed_cycles << ",\"sites\":[";
  bool first = true;
  for (const auto & [key, s] : r.sites)
  {
    j << (first ? "" : ",") << "{\"site\":\"" << json_escape(key) << "\",\"allocs\":" << s.allocs
      << ",\"bytes\":" << s.bytes << ",\"cycles\":" << s.cycles
      << ",\"first_cycle\":" << s.first_cycle << ",\"last_cycle\":" << s.last_cycle << "}";
    first = false;
  }
  j << "]}";
  std::cout << "[rtswe-json] " << j.str() << std::endl;
}

// Unguarded warm-up before the guarded cycles. One-time allocations happen
// here: lazy initialisation, and the first copy into a realtime publisher. A
// RealtimePublisher only takes a message when its thread is idle, so the
// sleeps make sure that first copy is not left to a guarded cycle. Returns
// the time stamp for the first guarded cycle, or nullopt if an update failed.
template <typename Update>
std::optional<rclcpp::Time> warm_up(
  Update && update, rclcpp::Time t, const rclcpp::Duration & period, int cycles = 3)
{
  for (int k = 0; k < cycles; ++k)
  {
    if (update(t, period) != controller_interface::return_type::OK) {return std::nullopt;}
    t = t + period;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return t;
}

// The hidden test's functional verdict, kept separate from the allocation
// verdict so the scorer can report F and R independently.
inline void report_functional(bool ok)
{
  std::cout << "[rtswe] functional_ok=" << (ok ? 1 : 0) << std::endl;
}

}  // namespace rtswe_eval

#endif  // RTSWE_EVAL__PROBE_HPP_
