// Positive control for the LV2 census: declares lv2:hardRTCapable and
// allocates in run() on purpose (a std::vector every run, malloc every 10th).
#include <lv2/core/lv2.h>
#include <cstdlib>
#include <vector>

namespace
{
struct Control
{
  const float * in = nullptr;
  float * out = nullptr;
  unsigned n = 0;
};

LV2_Handle instantiate(const LV2_Descriptor *, double, const char *, const LV2_Feature * const *)
{
  return new Control;
}
void connect_port(LV2_Handle h, uint32_t port, void * data)
{
  auto * c = static_cast<Control *>(h);
  if (port == 0) {c->in = static_cast<const float *>(data);} else {c->out = static_cast<float *>(data);}
}
void run(LV2_Handle h, uint32_t frames)
{
  auto * c = static_cast<Control *>(h);
  std::vector<float> tmp(c->in, c->in + frames);  // operator new
  if (++c->n % 10 == 0)
  {
    void * volatile p = std::malloc(64);          // malloc (volatile: not elided)
    std::free(p);
  }
  for (uint32_t i = 0; i < frames; ++i) {c->out[i] = 0.5f * tmp[i];}
}
void cleanup(LV2_Handle h) {delete static_cast<Control *>(h);}
const LV2_Descriptor descriptor = {"urn:rtswe:control-alloc", instantiate, connect_port, nullptr,
  run, nullptr, cleanup, nullptr};
}  // namespace

extern "C" LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor(uint32_t index)
{
  return index == 0 ? &descriptor : nullptr;
}
