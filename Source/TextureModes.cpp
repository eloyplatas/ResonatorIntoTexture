// Adaptador de TRIG DRUMS y DATA BENDER (mrarm84, MIT) para el plugin.
// Reproduce lo que hacen drum_mode.cc y bender_mode.cc del firmware, sin la
// capa de hardware (ADC, botones, anillos de audio): bloques de 32 muestras
// a 32 kHz, como en el módulo.
#include "TextureModes.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <cmath>
#include <cstring>

#include "clouds/bender/pn.h"
#include "clouds/bender/arena.h"
#include "clouds/drums/beat_tracker.h"
#include "clouds/drums/trig_drums.h"
#include "clouds/bender/bender_engine.h"
#include "clouds/perf/perf_fx.h"

namespace arena {
Bump M;
Bump C;
}  // namespace arena

namespace texmodes {

namespace {

std::mutex gMutex;
const void* gOwner[2] = {nullptr, nullptr};
int gActive = -1;  // motor cuyas memorias están montadas en la arena

std::vector<uint8_t>& memM() { static std::vector<uint8_t> m(1u << 20, 0); return m; }
std::vector<uint8_t>& memC() { static std::vector<uint8_t> c(1u << 19, 0); return c; }

bool gInit = false;
void initOnce() {
  if (gInit) return;
  arena::M.base = memM().data();
  arena::M.size = memM().size();
  arena::C.base = memC().data();
  arena::C.size = memC().size();
  bt::Init(32000.0f);
  tdr::Init(32000.0f);
  dbe::Init(32000.0f, 0x1234567u);
  gInit = true;
}

// Orden de los jacks del módulo -> voz del motor
const int kPageVoice[8] = {tdr::BD, tdr::SD, tdr::OH, tdr::CH, tdr::CP, tdr::MT, tdr::SA, tdr::SB};

// Reloj de charles cerrado (drum_mode.cc: HatClock): 1 bloque = 1 ms
uint32_t blockN = 0, hatAt = 0, hatIv[4] = {0, 0, 0, 0};
int hatIvN = 0, hatPpq = 2, hatCand = 2, hatCandN = 0, hatCount = 0, hatGateT = 0;

void hatClock() {
  const uint32_t iv = blockN - hatAt;
  hatAt = blockN;
  if (iv > 2000) hatIvN = 0, hatCount = 0;
  else hatIv[hatIvN & 3] = iv, hatIvN++;
  if (hatIvN >= 2) {
    uint32_t a[4];
    const int n = hatIvN < 4 ? hatIvN : 4;
    for (int i = 0; i < n; i++) a[i] = hatIv[i];
    for (int i = 1; i < n; i++)
      for (int j = i; j > 0 && a[j] < a[j - 1]; j--) { const uint32_t t = a[j]; a[j] = a[j - 1]; a[j - 1] = t; }
    const float unit = static_cast<float>(a[n / 2]) / 1000.0f;
    int want = hatPpq;
    for (int p = 1; p <= 4; p *= 2) {
      const float bpm = 60.0f / (unit * static_cast<float>(p));
      if (bpm >= 75.0f && bpm < 150.0f) want = p;
    }
    if (want != hatPpq) {
      hatCandN = want == hatCand ? hatCandN + 1 : 1;
      hatCand = want;
      if (hatCandN >= 4) hatPpq = want, hatCount = 0;
    } else {
      hatCandN = 0;
    }
  }
  if (hatCount % hatPpq == 0) hatGateT = 5;
  hatCount++;
}

Panel gPanel;
float gHit[8] = {};
bool gGrit = false;

dbe::Controls gBender;
float gSliceFlash = 0.0f;

void sanitize(float* l, float* r, size_t n, bool drums) {
  bool bad = false;
  for (size_t i = 0; i < n; i++)
    if (!(l[i] == l[i]) || !(r[i] == r[i]) || std::fabs(l[i]) > 4.0f || std::fabs(r[i]) > 4.0f) bad = true;
  if (!bad) return;
  if (drums) tdr::Panic(); else dbe::Reset();
  pfx::Reset();
  for (size_t i = 0; i < n; i++) l[i] = r[i] = 0.0f;
}

}  // namespace

bool acquire(int which, const void* owner) {
  std::lock_guard<std::mutex> lock(gMutex);
  initOnce();
  if (gOwner[which] != nullptr && gOwner[which] != owner) return false;
  // El otro motor comparte la arena: si lo usa otra instancia, no se puede.
  const int other = 1 - which;
  if (gOwner[other] != nullptr && gOwner[other] != owner) return false;
  gOwner[which] = owner;
  gOwner[other] = nullptr;
  return true;
}

void release(const void* owner) {
  std::lock_guard<std::mutex> lock(gMutex);
  for (auto& o : gOwner)
    if (o == owner) o = nullptr;
  if (gOwner[0] == nullptr && gOwner[1] == nullptr) gActive = -1;
}

// ------------------------------------------------------------- TRIG DRUMS

void drumsEnter() {
  initOnce();
  arena::M.Reset();
  arena::C.Reset();
  bt::Attach();
  tdr::Attach();
  tdr::Clear();
  tdr::Panic();
  pfx::Attach();
  gPanel = Panel();
  gPanel.a = false;
  gGrit = false;
  tdr::Activate(gPanel);
  gActive = DRUMS;
  blockN = hatAt = 0;
  hatIvN = hatCount = hatGateT = 0;
  hatPpq = hatCand = 2;
  hatCandN = 0;
}

void drumsProcess(const DrumControls& c, const float* inL, const float* inR,
                  float* outL, float* outR, size_t n) {
  if (gActive != DRUMS) drumsEnter();

  // Golpes (jacks / puertas)
  for (int p = 0; p < 8; ++p) {
    if (c.vel[p] > 0.0f) {
      const int v = kPageVoice[p];
      tdr::reqExt[v] = true;
      tdr::req[v] = c.vel[p];
    }
  }
  if (c.clockEdge) hatClock();
  blockN++;
  if (hatGateT > 0) hatGateT--;

  // Knobs: editan la voz seleccionada (salto al moverse)
  const int voice = kPageVoice[c.voice & 7];
  tdr::page = voice;
  float* pr = tdr::prm[voice];
  for (int k = 0; k < 6; ++k) {
    if (!c.moved[k]) continue;
    const float x = pn::Clamp(c.knob[k], 0.0f, 1.0f);
    switch (k) {
      case 0: pr[tdr::P_A] = x; break;
      case 1: pr[tdr::P_DECAY] = x; pr[tdr::P_SHAPE] = x; break;
      case 2: pr[tdr::P_TUNE] = x; break;
      case 3: pr[tdr::P_B] = x; break;
      case 4: pr[tdr::P_FILT] = x; break;
      default: pr[tdr::P_SEND] = x; break;
    }
  }
  tdr::panWidth = c.panWidth;
  tdr::echoFb = c.echoFb;
  tdr::rvTail = c.rvTail;
  tdr::dubReq = c.dub;
  tdr::throwReq = c.fx == 7;
  tdr::muteReq = false;

  float l[32], r[32], oL[32], oR[32];
  for (size_t off = 0; off < n; off += 32) {
    const size_t m = std::min<size_t>(32, n - off);
    for (size_t i = 0; i < m; ++i) { l[i] = inL[off + i]; r[i] = inR[off + i]; }
    bt::Process(l, r, m, hatGateT > 0);
    bt::freeBpm = c.tempo;
    gPanel.row = c.kit;
    gPanel.col = c.audioIn;
    const float* ip[2] = {l, r};
    float* op[2] = {oL, oR};
    tdr::Process(gPanel, ip, op, m);

    pfx::Ctl fc;
    fc.beat = bt::st.period;
    fc.phase = bt::st.phase;
    pfx::Preset(c.fx >= 0 ? c.fx : -1, true, fc.beat, m, fc);
    if (c.fx < 0)
      for (int b = 0; b < 3; b++)
        for (int k = 0; k < 6; k++) fc.amt[b][k] = 0.0f;
    pfx::Process(fc, oL, oR, m);
    sanitize(oL, oR, m, true);
    for (size_t i = 0; i < m; ++i) { outL[off + i] = oL[i]; outR[off + i] = oR[i]; }
  }
  bt::Update();

  for (int p = 0; p < 8; ++p) {
    const int v = kPageVoice[p];
    const float h = tdr::hitShow[v];
    tdr::hitShow[v] = 0.0f;
    gHit[p] = pn::Max(gHit[p] * 0.97f, h);
  }
}

float drumsHit(int jackVoice) { return gHit[jackVoice & 7]; }

void drumsGetVoiceParams(float* dst) {
  std::memcpy(dst, tdr::prm, sizeof(float) * kDrumParams);
}
void drumsDefaultParams(float* dst) {
  std::memcpy(dst, tdr::kPrmDef, sizeof(float) * kDrumParams);
}
void drumsSetVoiceParams(const float* src) {
  std::memcpy(tdr::prm, src, sizeof(float) * kDrumParams);
}

// ------------------------------------------------------------ DATA BENDER

void benderEnter() {
  initOnce();
  arena::M.Reset();
  arena::C.Reset();
  dbe::Attach();
  pfx::Attach();
  dbe::Reset();
  gActive = BENDER;
}

void benderProcess(const BenderControls& c, const float* inL, const float* inR,
                   float* outL, float* outR, size_t n) {
  if (gActive != BENDER) benderEnter();
  gBender.time = c.time;
  gBender.repeats = c.repeats;
  gBender.bend = c.bend;
  gBender.extraOct = c.extraOct;
  gBender.brk = c.brk;
  gBender.corrupt = c.corrupt;
  gBender.mix = c.mix;
  gBender.mode = static_cast<dbe::EngineMode>(c.mode % dbe::MODE_LAST);
  gBender.corruptType = c.corruptType % dbe::C_LAST;
  gBender.reverse = c.reverse;
  gBender.freeze = c.freeze;
  gBender.windowMs = 10.0f;
  gBender.shuffle = 0.0f;
  gBender.reverseChance = c.reverseChance;
  gBender.breakShape = 0.5f;
  gBender.regen = 0.9f * c.regen;
  gBender.width = 2.0f * c.width;
  gBender.unique = c.width > 0.75f;
  gBender.gate = c.clockEdge;

  float l[32], r[32], oL[32], oR[32];
  for (size_t off = 0; off < n; off += 32) {
    const size_t m = std::min<size_t>(32, n - off);
    for (size_t i = 0; i < m; ++i) { l[i] = inL[off + i]; r[i] = inR[off + i]; }
    dbe::Process(gBender, l, r, oL, oR, m);
    gBender.gate = false;

    pfx::Ctl fc;
    fc.beat = dbe::clocked ? static_cast<float>(dbe::clockPeriod) : static_cast<float>(dbe::T);
    fc.phase = dbe::clocked ? 0.0f : static_cast<float>(dbe::t) / static_cast<float>(dbe::T > 0 ? dbe::T : 1);
    if (dbe::clocked) {
      const float p = static_cast<float>(dbe::sampleClock - dbe::lastEdge) / fc.beat;
      fc.phase = p - pn::Floor(p);
    }
    pfx::Preset(c.fx >= 0 ? c.fx : -1, false, fc.beat, m, fc);
    if (c.fx < 0)
      for (int b = 0; b < 3; b++)
        for (int k = 0; k < 6; k++) fc.amt[b][k] = 0.0f;
    pfx::Process(fc, oL, oR, m);
    sanitize(oL, oR, m, false);
    for (size_t i = 0; i < m; ++i) { outL[off + i] = oL[i]; outR[off + i] = oR[i]; }
  }
  if (dbe::status.sliceFlash > 0.5f) { dbe::status.sliceFlash = 0.0f; gSliceFlash = 1.0f; }
  else gSliceFlash *= 0.95f;
}

float benderSliceFlash() { return gSliceFlash; }

}  // namespace texmodes
