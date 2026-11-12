// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// LV2 census: arm malloc-guard around LV2_Descriptor::run() of one plugin and
// report every heap allocation. LV2 requires plugins that declare
// lv2:hardRTCapable not to use malloc() or free() in run().
//
//   lv2_census <plugin-uri>   -> one JSON line on stdout
//
// The plugin is instantiated at 48 kHz with block length 256, every port is
// connected (audio and CV inputs carry a 440 Hz sine, control inputs their
// default, atom inputs an event sequence with MIDI notes for plugins that take
// MIDI). After 16 unguarded warm-up runs (with one note on and off), 400 runs
// are guarded; a note starts in run 20 and ends in run 200. From guarded run 25
// on, every 50 runs, every control input jumps to a pseudo-random valid value
// (fixed seed; integer, toggle and enumeration ports get valid values), as a
// host automating parameters would; the change is made outside the guard. The host's own
// features (URID map, log, options, worker) never allocate, so every
// allocation recorded inside run() is the plugin's. Worker requests are
// executed between runs, outside the guard. Only run() is guarded.

#include <lilv/lilv.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/core/lv2.h>
#include <lv2/log/log.h>
#include <lv2/midi/midi.h>
#include <lv2/options/options.h>
#include <lv2/parameters/parameters.h>
#include <lv2/state/state.h>
#include <lv2/urid/urid.h>
#include <lv2/worker/worker.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "rtswe_eval/guarded_cycle.hpp"

namespace
{
constexpr uint32_t kBlock = 256;
constexpr double kRate = 48000.0;
constexpr int kWarm = 16;
constexpr int kRuns = 400;
constexpr uint32_t kAtomCap = 65536;

// ---------------------------------------------------------------- URID map
// Fixed storage: mapping a known URI never allocates, a new one copies into a pool.
struct Urids
{
  static constexpr std::size_t kMax = 4096;
  static constexpr std::size_t kPool = 1 << 20;
  char pool[kPool];
  std::size_t used = 0;
  const char * uris[kMax];
  std::size_t n = 0;

  LV2_URID map(const char * uri)
  {
    for (std::size_t i = 0; i < n; ++i)
    {
      if (std::strcmp(uris[i], uri) == 0) {return static_cast<LV2_URID>(i + 1);}
    }
    const std::size_t len = std::strlen(uri) + 1;
    if (n == kMax || used + len > kPool) {return 0;}
    std::memcpy(pool + used, uri, len);
    uris[n] = pool + used;
    used += len;
    return static_cast<LV2_URID>(++n);
  }
  const char * unmap(LV2_URID id) const {return id >= 1 && id <= n ? uris[id - 1] : nullptr;}
};
Urids g_urids;

LV2_URID urid_map(LV2_URID_Map_Handle, const char * uri) {return g_urids.map(uri);}
const char * urid_unmap(LV2_URID_Unmap_Handle, LV2_URID id) {return g_urids.unmap(id);}

int log_printf(LV2_Log_Handle, LV2_URID, const char *, ...) {return 0;}
int log_vprintf(LV2_Log_Handle, LV2_URID, const char *, va_list) {return 0;}

// ---------------------------------------------------------------- worker
// Requests from run() are copied into fixed storage and executed between runs.
struct Queue
{
  static constexpr std::size_t kCap = 1 << 16;
  uint8_t buf[kCap];
  std::size_t used = 0;
  bool push(uint32_t size, const void * data)
  {
    if (used + sizeof(uint32_t) + size > kCap) {return false;}
    std::memcpy(buf + used, &size, sizeof(uint32_t));
    std::memcpy(buf + used + sizeof(uint32_t), data, size);
    used += sizeof(uint32_t) + size;
    return true;
  }
  template <typename F>
  void drain(F && f)
  {
    std::size_t off = 0;
    while (off < used)
    {
      uint32_t size;
      std::memcpy(&size, buf + off, sizeof(uint32_t));
      f(size, buf + off + sizeof(uint32_t));
      off += sizeof(uint32_t) + size;
    }
    used = 0;
  }
};
Queue g_requests, g_responses;

LV2_Worker_Status schedule_work(LV2_Worker_Schedule_Handle, uint32_t size, const void * data)
{
  return g_requests.push(size, data) ? LV2_WORKER_SUCCESS : LV2_WORKER_ERR_NO_SPACE;
}
LV2_Worker_Status respond(LV2_Worker_Respond_Handle, uint32_t size, const void * data)
{
  return g_responses.push(size, data) ? LV2_WORKER_SUCCESS : LV2_WORKER_ERR_NO_SPACE;
}

std::string json(const std::string & s)
{
  std::string o = "\"";
  for (char c : s)
  {
    if (c == '"' || c == '\\') {o += '\\'; o += c;}
    else if (static_cast<unsigned char>(c) < 0x20) {o += ' ';}
    else {o += c;}
  }
  return o + "\"";
}

struct Site
{
  std::size_t allocs = 0, bytes = 0, runs = 0;
  int first = -1, last = -1;
};
}  // namespace

int main(int argc, char ** argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: lv2_census <plugin-uri>\n";
    return 2;
  }
  const std::string uri_str = argv[1];
  std::ostringstream out;
  out << "{\"uri\":" << json(uri_str);
  auto finish = [&](const std::string & status) {
    out << ",\"status\":" << json(status) << "}";
    std::cout << out.str() << std::endl;
  };

  LilvWorld * world = lilv_world_new();
  lilv_world_load_all(world);
  LilvNode * uri = lilv_new_uri(world, uri_str.c_str());
  const LilvPlugin * plugin = lilv_plugins_get_by_uri(lilv_world_get_all_plugins(world), uri);
  if (!plugin)
  {
    finish("error: plugin not found");
    return 0;
  }
  LilvNode * name = lilv_plugin_get_name(plugin);
  LilvNode * hard_rt = lilv_new_uri(world, LV2_CORE__hardRTCapable);
  const bool is_hard_rt = lilv_plugin_has_feature(plugin, hard_rt);
  out << ",\"name\":" << json(name ? lilv_node_as_string(name) : "")
      << ",\"bundle\":" << json(lilv_node_as_uri(lilv_plugin_get_bundle_uri(plugin)))
      << ",\"hard_rt\":" << (is_hard_rt ? "true" : "false");

  // ---- host features
  LV2_URID_Map map{nullptr, urid_map};
  LV2_URID_Unmap unmap{nullptr, urid_unmap};
  LV2_Log_Log log{nullptr, log_printf, log_vprintf};
  LV2_Worker_Schedule schedule{nullptr, schedule_work};
  int32_t block = kBlock, seq_size = kAtomCap;
  float rate = static_cast<float>(kRate);
  const LV2_URID atom_int = g_urids.map(LV2_ATOM__Int), atom_float = g_urids.map(LV2_ATOM__Float);
  LV2_Options_Option options[] = {
    {LV2_OPTIONS_INSTANCE, 0, g_urids.map(LV2_BUF_SIZE__minBlockLength), sizeof(int32_t), atom_int, &block},
    {LV2_OPTIONS_INSTANCE, 0, g_urids.map(LV2_BUF_SIZE__maxBlockLength), sizeof(int32_t), atom_int, &block},
    {LV2_OPTIONS_INSTANCE, 0, g_urids.map(LV2_BUF_SIZE__nominalBlockLength), sizeof(int32_t), atom_int, &block},
    {LV2_OPTIONS_INSTANCE, 0, g_urids.map(LV2_BUF_SIZE__sequenceSize), sizeof(int32_t), atom_int, &seq_size},
    {LV2_OPTIONS_INSTANCE, 0, g_urids.map(LV2_PARAMETERS__sampleRate), sizeof(float), atom_float, &rate},
    {LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr}};
  const LV2_Feature f_map{LV2_URID__map, &map}, f_unmap{LV2_URID__unmap, &unmap},
    f_log{LV2_LOG__log, &log}, f_sched{LV2_WORKER__schedule, &schedule},
    f_opts{LV2_OPTIONS__options, options}, f_bounded{LV2_BUF_SIZE__boundedBlockLength, nullptr},
    f_fixed{LV2_BUF_SIZE__fixedBlockLength, nullptr}, f_pow2{LV2_BUF_SIZE__powerOf2BlockLength, nullptr},
    f_default_state{LV2_STATE__loadDefaultState, nullptr}, f_live{LV2_CORE__isLive, nullptr};
  const LV2_Feature * features[] = {&f_map, &f_unmap, &f_log, &f_sched, &f_opts, &f_bounded,
    &f_fixed, &f_pow2, &f_default_state, &f_live, nullptr};
  std::set<std::string> supported;
  for (const LV2_Feature * const * f = features; *f; ++f) {supported.insert((*f)->URI);}

  LilvNodes * required = lilv_plugin_get_required_features(plugin);
  LILV_FOREACH(nodes, i, required)
  {
    const std::string req = lilv_node_as_uri(lilv_nodes_get(required, i));
    if (!supported.count(req))
    {
      finish("skip: requires " + req);
      return 0;
    }
  }

  // ---- ports and buffers
  LilvNode * n_audio = lilv_new_uri(world, LV2_CORE__AudioPort);
  LilvNode * n_cv = lilv_new_uri(world, LV2_CORE__CVPort);
  LilvNode * n_control = lilv_new_uri(world, LV2_CORE__ControlPort);
  LilvNode * n_atom = lilv_new_uri(world, LV2_ATOM__AtomPort);
  LilvNode * n_input = lilv_new_uri(world, LV2_CORE__InputPort);
  LilvNode * n_optional = lilv_new_uri(world, LV2_CORE__connectionOptional);
  LilvNode * n_midi = lilv_new_uri(world, LV2_MIDI__MidiEvent);
  LilvNode * n_integer = lilv_new_uri(world, LV2_CORE__integer);
  LilvNode * n_toggled = lilv_new_uri(world, LV2_CORE__toggled);
  LilvNode * n_enum = lilv_new_uri(world, LV2_CORE__enumeration);

  const uint32_t nports = lilv_plugin_get_num_ports(plugin);
  std::vector<float> mins(nports), maxs(nports), defs(nports);
  lilv_plugin_get_port_ranges_float(plugin, mins.data(), maxs.data(), defs.data());
  std::vector<std::vector<float>> audio(nports);
  std::vector<float> control(nports, 0.0f);
  std::vector<std::vector<uint64_t>> atoms(nports);
  std::vector<uint32_t> audio_in, atom_in, atom_out;
  std::vector<bool> midi_in(nports, false);
  enum Kind {kNone, kAudio, kControl, kAtom};
  std::vector<Kind> kind(nports, kNone);
  // automation: per control input, the values it may take (empty: continuous in [min, max])
  std::vector<uint32_t> control_in;
  std::vector<std::vector<float>> choices(nports);
  std::vector<bool> integer(nports, false);
  for (uint32_t p = 0; p < nports; ++p)
  {
    const LilvPort * port = lilv_plugin_get_port_by_index(plugin, p);
    const bool in = lilv_port_is_a(plugin, port, n_input);
    if (lilv_port_is_a(plugin, port, n_audio) || lilv_port_is_a(plugin, port, n_cv))
    {
      kind[p] = kAudio;
      audio[p].assign(kBlock, 0.0f);
      if (in) {audio_in.push_back(p);}
    }
    else if (lilv_port_is_a(plugin, port, n_control))
    {
      kind[p] = kControl;
      float v = defs[p];
      if (std::isnan(v)) {v = std::isnan(mins[p]) ? 0.0f : mins[p];}
      control[p] = v;
      if (in && std::isfinite(mins[p]) && std::isfinite(maxs[p]) && maxs[p] > mins[p])
      {
        control_in.push_back(p);
        if (lilv_port_has_property(plugin, port, n_toggled)) {choices[p] = {mins[p], maxs[p]};}
        else if (lilv_port_has_property(plugin, port, n_enum))
        {
          LilvScalePoints * points = lilv_port_get_scale_points(plugin, port);
          LILV_FOREACH(scale_points, i, points)
          {
            choices[p].push_back(lilv_node_as_float(lilv_scale_point_get_value(lilv_scale_points_get(points, i))));
          }
          lilv_scale_points_free(points);
        }
        integer[p] = lilv_port_has_property(plugin, port, n_integer);
      }
    }
    else if (lilv_port_is_a(plugin, port, n_atom))
    {
      kind[p] = kAtom;
      atoms[p].assign(kAtomCap / sizeof(uint64_t), 0);
      (in ? atom_in : atom_out).push_back(p);
      midi_in[p] = in && lilv_port_supports_event(plugin, port, n_midi);
    }
    else if (!lilv_port_has_property(plugin, port, n_optional))
    {
      finish("skip: unsupported port type on port " + std::to_string(p));
      return 0;
    }
  }
  const bool takes_midi = std::find(midi_in.begin(), midi_in.end(), true) != midi_in.end();
  out << ",\"ports\":" << nports << ",\"midi\":" << (takes_midi ? "true" : "false");

  LilvInstance * instance = lilv_plugin_instantiate(plugin, kRate, features);
  if (!instance)
  {
    finish("error: instantiate failed");
    return 0;
  }
  for (uint32_t p = 0; p < nports; ++p)
  {
    void * buf = nullptr;
    if (kind[p] == kAudio) {buf = audio[p].data();}
    else if (kind[p] == kControl) {buf = &control[p];}
    else if (kind[p] == kAtom) {buf = atoms[p].data();}
    lilv_instance_connect_port(instance, p, buf);
  }
  const auto * worker = static_cast<const LV2_Worker_Interface *>(
    lilv_instance_get_extension_data(instance, LV2_WORKER__interface));
  if (lilv_plugin_has_extension_data(plugin, lilv_new_uri(world, LV2_WORKER__interface)) && !worker)
  {
    finish("error: worker interface missing");
    return 0;
  }
  const LV2_URID seq_type = g_urids.map(LV2_ATOM__Sequence), chunk_type = g_urids.map(LV2_ATOM__Chunk),
    midi_type = g_urids.map(LV2_MIDI__MidiEvent);

  lilv_instance_activate(instance);
  // After instantiate: the plugin library is loaded now, so its GOT gets hooked too.
  rtswe_eval::install_hooks();

  std::map<std::string, Site> sites;
  std::size_t total_allocs = 0, total_bytes = 0, alloc_runs = 0;
  int first_run = -1, last_run = -1;
  double phase = 0.0;
  uint64_t seed = 0x2545F4914F6CDD1DULL;
  auto uniform = [&seed]() {  // xorshift, deterministic
    seed ^= seed << 13; seed ^= seed >> 7; seed ^= seed << 17;
    return static_cast<double>(seed >> 11) / static_cast<double>(1ULL << 53);
  };
  int automation_steps = 0;
  for (int k = 0; k < kWarm + kRuns; ++k)
  {
    const int g = k - kWarm;  // guarded run index, negative during warm-up
    if (g >= 25 && (g - 25) % 50 == 0)
    {
      ++automation_steps;
      for (uint32_t p : control_in)
      {
        if (!choices[p].empty())
        {
          control[p] = choices[p][static_cast<std::size_t>(uniform() * choices[p].size()) % choices[p].size()];
        }
        else
        {
          double v = mins[p] + uniform() * (maxs[p] - mins[p]);
          control[p] = static_cast<float>(integer[p] ? std::round(v) : v);
        }
      }
    }
    for (uint32_t p : audio_in)
    {
      for (uint32_t i = 0; i < kBlock; ++i)
      {
        audio[p][i] = 0.25f * static_cast<float>(std::sin(phase + 2.0 * M_PI * 440.0 * i / kRate));
      }
    }
    phase = std::fmod(phase + 2.0 * M_PI * 440.0 * kBlock / kRate, 2.0 * M_PI);
    for (uint32_t p : atom_in)
    {
      auto * seq = reinterpret_cast<LV2_Atom_Sequence *>(atoms[p].data());
      seq->atom.type = seq_type;
      seq->atom.size = sizeof(LV2_Atom_Sequence_Body);
      seq->body.unit = 0;
      seq->body.pad = 0;
      const bool on = k == 4 || g == 20, off = k == 10 || g == 200;
      if (midi_in[p] && (on || off))
      {
        struct
        {
          LV2_Atom_Event ev;
          uint8_t msg[3];
        } e{};
        e.ev.time.frames = 0;
        e.ev.body.type = midi_type;
        e.ev.body.size = 3;
        e.msg[0] = on ? 0x90 : 0x80;
        e.msg[1] = 60;
        e.msg[2] = on ? 100 : 0;
        lv2_atom_sequence_append_event(seq, kAtomCap - sizeof(LV2_Atom), &e.ev);
      }
    }
    for (uint32_t p : atom_out)
    {
      auto * a = reinterpret_cast<LV2_Atom *>(atoms[p].data());
      a->type = chunk_type;
      a->size = kAtomCap - sizeof(LV2_Atom);
    }

    if (g < 0)
    {
      lilv_instance_run(instance, kBlock);
    }
    else
    {
      rtswe_eval::CycleReport r;
      rtswe_eval::guarded([&] {lilv_instance_run(instance, kBlock);}, r);
      if (r.violations > 0)
      {
        ++alloc_runs;
        total_allocs += r.violations;
        total_bytes += r.bytes;
        if (first_run < 0) {first_run = g;}
        last_run = g;
        std::set<std::string> seen;
        for (const auto & a : r.allocs)
        {
          const std::string s = rtswe_eval::site_of(a.trace);
          Site & site = sites[s];
          ++site.allocs;
          site.bytes += a.bytes;
          if (seen.insert(s).second)
          {
            ++site.runs;
            if (site.first < 0) {site.first = g;}
            site.last = g;
          }
        }
      }
    }
    if (worker)
    {
      g_requests.drain([&](uint32_t size, const void * data) {
        worker->work(lilv_instance_get_handle(instance), respond, nullptr, size, data);
      });
      g_responses.drain([&](uint32_t size, const void * data) {
        worker->work_response(lilv_instance_get_handle(instance), size, data);
      });
      if (worker->end_run) {worker->end_run(lilv_instance_get_handle(instance));}
    }
  }
  lilv_instance_deactivate(instance);

  out << ",\"automated_controls\":" << control_in.size() << ",\"automation_steps\":" << automation_steps;
  out << ",\"runs\":" << kRuns << ",\"allocating_runs\":" << alloc_runs << ",\"allocs\":" << total_allocs
      << ",\"bytes\":" << total_bytes << ",\"first_run\":" << first_run << ",\"last_run\":" << last_run
      << ",\"sites\":[";
  bool first = true;
  for (const auto & [s, site] : sites)
  {
    out << (first ? "" : ",") << "{\"site\":" << json(s) << ",\"allocs\":" << site.allocs
        << ",\"bytes\":" << site.bytes << ",\"runs\":" << site.runs << ",\"first_run\":" << site.first
        << ",\"last_run\":" << site.last << "}";
    first = false;
  }
  out << "]";
  finish("ok");
  std::_Exit(0);  // skip plugin teardown: some plugins crash on cleanup, and it is not measured
}
