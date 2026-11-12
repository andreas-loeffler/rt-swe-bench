// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Tier 1 test helper: runs a kernel's real-time call under malloc-guard, cycle by
// cycle, with the same recorder as the repository tier (rtswe_eval), and prints
// the result lines the scoring script reads:
//   [rtswe] functional_ok=<0|1>
//   [rtswe-json] {"cycles":..,"allocating_cycles":..,"allocs":..,"bytes":..,"sites":[..]}

#ifndef RTK__CHECK_HPP_
#define RTK__CHECK_HPP_

#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include "rtswe_eval/guarded_cycle.hpp"

namespace rtk
{

struct Run
{
  int cycles = 0;
  int allocating_cycles = 0;
  std::size_t allocs = 0;
  std::size_t bytes = 0;
  std::map<std::string, std::size_t> sites;  // site -> allocations
  bool functional = true;
  std::string first_mismatch;
  Run() { active() = this; }
  ~Run() { if (active() == this) {active() = nullptr;} }
  Run(const Run &) = delete;
  Run & operator=(const Run &) = delete;
  // The most recently created run, for labelled checks that do not pass one.
  static Run *& active() { static Run * r = nullptr; return r; }
};

inline void setup() { rtswe_eval::install_hooks(); }

// Runs f() under the guard and records what it allocated. Returns f()'s value.
template <typename F>
auto guarded(Run & run, F && f)
{
  rtswe_eval::CycleReport r;
  auto record = [&] {
    ++run.cycles;
    if (r.violations > 0)
    {
      ++run.allocating_cycles;
      run.allocs += r.violations;
      run.bytes += r.bytes;
      for (const auto & a : r.allocs) {++run.sites[rtswe_eval::site_of(a.trace)];}
    }
  };
  if constexpr (std::is_void_v<std::invoke_result_t<F &>>)
  {  // a call without a return value is accepted here too
    rtswe_eval::guarded([&] { f(); return 0; }, r);
    record();
  }
  else
  {
    auto value = rtswe_eval::guarded(std::forward<F>(f), r);
    record();
    return value;
  }
}

// For calls without a return value.
template <typename F>
void guarded_void(Run & run, F && f)
{
  guarded(run, [&] { f(); return 0; });
}

inline void expect(Run & run, bool ok, const std::string & what = "check")
{
  if (!ok && run.functional)
  {
    run.functional = false;
    run.first_mismatch = what;
  }
}

inline bool near(double a, double b, double tol = 1e-9)
{
  return std::abs(a - b) <= tol * (1.0 + std::abs(b));
}

// near() that also records the comparison as a functional check, for tests that pass the run.
inline bool near(Run & run, double a, double b, double tol = 1e-9, const std::string & what = "")
{
  const bool ok = near(a, b, tol);
  if (!ok) {expect(run, false, what.empty() ? "near(" + std::to_string(a) + ", " + std::to_string(b) + ")" : what);}
  return ok;
}

// A labelled near() without a run records into the active run.
inline bool near(double a, double b, double tol, const std::string & what)
{
  const bool ok = near(a, b, tol);
  if (!ok && Run::active() != nullptr) {expect(*Run::active(), false, what);}
  return ok;
}

inline std::string json_escape(const std::string & s)
{
  std::string o;
  for (char c : s)
  {
    if (c == '"' || c == '\\') {o += '\\'; o += c;}
    else if (static_cast<unsigned char>(c) < 0x20) {o += ' ';}
    else {o += c;}
  }
  return o;
}

inline int report(const Run & run)
{
  std::cout << "[rtswe] functional_ok=" << (run.functional ? 1 : 0);
  if (!run.functional) {std::cout << " first_mismatch=" << run.first_mismatch;}
  std::cout << "\n[rtswe-json] {\"cycles\":" << run.cycles << ",\"allocating_cycles\":" << run.allocating_cycles
            << ",\"allocs\":" << run.allocs << ",\"bytes\":" << run.bytes << ",\"sites\":[";
  bool first = true;
  for (const auto & [site, n] : run.sites)
  {
    std::cout << (first ? "" : ",") << "{\"site\":\"" << json_escape(site) << "\",\"allocs\":" << n << "}";
    first = false;
  }
  std::cout << "]}" << std::endl;
  return 0;
}

}  // namespace rtk

#endif  // RTK__CHECK_HPP_
