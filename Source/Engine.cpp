#include "Engine.h"

#include <cmath>
#include <cstring>
#include <chrono>

#include "rings/dsp/part.h"
#include "rings/dsp/strummer.h"
#include "rings/dsp/patch.h"
#include "rings/dsp/performance_state.h"

#include "clouds/dsp/granular_processor.h"

#include "plaits/dsp/voice.h"

#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"
#include "marbles/random/random_generator.h"
#include "marbles/random/random_stream.h"
#include "marbles/ramp/ramp.h"

#include "stmlib/dsp/hysteresis_quantizer.h"
#include "stmlib/utils/buffer_allocator.h"
#include "stmlib/utils/gate_flags.h"
#include "Scales.h"
#include "TextureModes.h"

namespace ric {

using namespace port;

namespace {

constexpr int kB = Engine::kBlock;  // 24 muestras a 48 kHz
constexpr int kPlaitsBlock = 12;
constexpr int kCloudsBlock = 32;
constexpr int kCloudsPrime = 136;   // muestras a 48 kHz en la FIFO de salida de Clouds

const marbles::Scale kPresetScales[6] = {
  {1.0f, 12, {{0.0000f, 255}, {0.0833f, 16}, {0.1667f, 96}, {0.2500f, 24},
              {0.3333f, 128}, {0.4167f, 64}, {0.5000f, 8}, {0.5833f, 192},
              {0.6667f, 16}, {0.7500f, 96}, {0.8333f, 24}, {0.9167f, 128}}},
  {1.0f, 12, {{0.0000f, 255}, {0.0833f, 16}, {0.1667f, 96}, {0.2500f, 128},
              {0.3333f, 8}, {0.4167f, 64}, {0.5000f, 4}, {0.5833f, 192},
              {0.6667f, 96}, {0.7500f, 16}, {0.8333f, 128}, {0.9167f, 16}}},
  {1.0f, 12, {{0.0000f, 255}, {0.0833f, 4}, {0.1667f, 96}, {0.2500f, 4},
              {0.3333f, 4}, {0.4167f, 140}, {0.5000f, 4}, {0.5833f, 192},
              {0.6667f, 4}, {0.7500f, 96}, {0.8333f, 4}, {0.9167f, 4}}},
  {1.0f, 7, {{0.0000f, 255}, {0.1275f, 128}, {0.2625f, 32}, {0.4600f, 8},
             {0.5883f, 192}, {0.7067f, 64}, {0.8817f, 16}}},
  {1.0f, 12, {{0.0000f, 255}, {0.0752f, 128}, {0.1699f, 4}, {0.2630f, 4},
              {0.3219f, 128}, {0.4150f, 64}, {0.4918f, 4}, {0.5850f, 192},
              {0.6601f, 64}, {0.7549f, 4}, {0.8479f, 4}, {0.9069f, 64}}},
  {1.0f, 12, {{0.0000f, 255}, {0.0752f, 4}, {0.1699f, 128}, {0.2630f, 64},
              {0.3219f, 4}, {0.4150f, 128}, {0.4918f, 4}, {0.5850f, 192},
              {0.6601f, 4}, {0.7549f, 64}, {0.8479f, 128}, {0.9069f, 4}}},
};

const int kLoopLength[] = {
  1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7,
  8, 8, 8, 8, 8, 8, 8, 8, 8,
  10, 10, 10,
  12, 12, 12, 12, 12, 12, 12,
  14, 14,
  16
};

inline float clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }

inline float softClip(float x) {
  if (x > 3.0f) return 1.0f;
  if (x < -3.0f) return -1.0f;
  const float x2 = x * x;
  return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline int16_t toShort(float x) { return static_cast<int16_t>(softClip(x) * 32767.0f); }

}  // namespace

struct Engine::Impl {
  // Rings
  rings::Part part;
  rings::Strummer strummer;
  std::vector<uint16_t> ringsReverb = std::vector<uint16_t>(32768, 0);
  float gateLevel = 0.0f;
  int chord = 0;
  float prevStrum = 0.0f;

  // Clouds (subdominio 32 kHz)
  clouds::GranularProcessor clouds;
  std::vector<uint8_t> cloudsLarge = std::vector<uint8_t>(118784, 0);
  std::vector<uint8_t> cloudsSmall = std::vector<uint8_t>(65536 - 128, 0);
  StreamResampler rs48to32, rs32to48;
  Fifo cIn32, cOut48;
  bool cloudsTrig = false;
  float prevCTrig = 0.0f;

  // Plaits
  plaits::Voice plaits;
  std::vector<char> plaitsMem = std::vector<char>(32768, 0);

  // Marbles
  marbles::RandomGenerator rng;
  marbles::RandomStream stream;
  marbles::TGenerator tGen;
  marbles::XYGenerator xyGen;
  stmlib::HysteresisQuantizer2 dvLengthQ;
  float rampBuffer[kB * 4];
  bool gates[kB * 2];
  float voltages[kB * 4];
  stmlib::GateFlags noClock[kB];

  // Seguidor de envolvente de la entrada
  float env = 0.0f;
  float mixEff = 1.0f;
};

Engine::Engine() : impl_(std::make_unique<Impl>()) {
  texmodes::drumsDefaultParams(drumPrm_);
}
Engine::~Engine() { texmodes::release(this); }

void Engine::getDrumParams(float* dst) const { std::memcpy(dst, drumPrm_, sizeof(drumPrm_)); }
void Engine::setDrumParams(const float* src) {
  std::memcpy(drumPrm_, src, sizeof(drumPrm_));
  drumPrmLoaded_ = false;  // se vuelcan al motor en el siguiente bloque
}

float Engine::quantize(float note, int mode, const EngineParams& p) const {
  if (mode <= 0) return note;
  if (mode == 1) return std::round(note);
  // Escala activa del random sampler (o la aprendida por MIDI)
  const bool midi = p.scale >= scales::midiIndex();
  int root = p.root;
  float degrees[16];
  int nd = 0;
  if (midi && learnedMask_ != 0 && learnedRoot_ >= 0) {
    root = learnedRoot_;
    for (int i = 0; i < 12; ++i)
      if (learnedMask_ & (1 << ((root + i) % 12))) degrees[nd++] = static_cast<float>(i);
  } else if (!midi) {
    const auto& d = scales::all()[static_cast<size_t>(std::max(0, p.scale))];
    for (size_t k = 0; k < d.semis.size() && nd < 16; ++k)
      if (d.weights.empty() || d.weights[k] >= 64) degrees[nd++] = d.semis[k];
  }
  if (nd == 0) return std::round(note);
  const float rel = note - static_cast<float>(root);
  const float oct = std::floor(rel / 12.0f);
  const float frac = rel - oct * 12.0f;
  float best = degrees[0], bestDist = 1e9f;
  for (int i = 0; i < nd; ++i)
    for (float o : {-12.0f, 0.0f, 12.0f}) {
      const float dd = std::abs(frac - (degrees[i] + o));
      if (dd < bestDist) { bestDist = dd; best = degrees[i] + o; }
    }
  return static_cast<float>(root) + oct * 12.0f + best;
}

void Engine::prepare(double hostRate, int maxBlock) {
  hostRate_ = hostRate;
  auto& m = *impl_;

  m.part.Init(m.ringsReverb.data());
  m.strummer.Init(0.01f, rings::kSampleRate / rings::kMaxBlockSize);
  m.gateLevel = 0.0f;
  m.chord = 0;

  m.clouds.Init(m.cloudsLarge.data(), m.cloudsLarge.size(),
                m.cloudsSmall.data(), m.cloudsSmall.size());
  m.clouds.set_playback_mode(clouds::PLAYBACK_MODE_GRANULAR);
  m.clouds.set_quality(0);
  m.clouds.Prepare();
  m.rs48to32.init(48000.0, 32000.0, 2);
  m.rs32to48.init(32000.0, 48000.0, 2);
  m.cIn32.init(2, 4096);
  m.cOut48.init(2, 4096);
  m.cOut48.pushSilence(kCloudsPrime);

  stmlib::BufferAllocator allocator(m.plaitsMem.data(), m.plaitsMem.size());
  m.plaits.Init(&allocator);

  const auto seed = static_cast<uint32_t>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  m.rng.Init(seed | 1u);
  m.stream.Init(&m.rng);
  m.tGen.Init(&m.stream, 48000.0f);
  m.xyGen.Init(&m.stream, 48000.0f);
  for (int i = 0; i < 6; ++i) m.xyGen.LoadScale(i, kPresetScales[i]);
  m.dvLengthQ.Init(sizeof(kLoopLength) / sizeof(int), 0.25f, false);
  std::memset(m.noClock, 0, sizeof(m.noClock));
  loadedScaleKey_ = -999;
  scaleDirty_ = true;

  rsIn_.init(hostRate, 48000.0, 2);
  rsOut_.init(48000.0, hostRate, 2);
  const int cap = 16384 + maxBlock * 4;
  in48_.init(2, cap);
  outHost_.init(2, cap + 4096);

  const double h = hostRate;
  const int K = StreamResampler::kHalf;
  const int prime = static_cast<int>(std::ceil(K + 1 + h * (kB + K + 1) / 48000.0)) + 32;
  outHost_.pushSilence(prime);

  // Latencia del camino normalizado (pasa por Clouds).
  latency_ = prime + K + static_cast<int>(std::lround(h * K / 48000.0)) +
             static_cast<int>(std::lround(h * (kCloudsPrime + K + K * 1.5) / 48000.0));

  for (auto& d : dryDelay_) d.assign(static_cast<size_t>(latency_ + 1), 0.0f);
  dryWrite_ = 0;
  for (auto& b : outBuf_) std::fill(b, b + kB, 0.0f);

  tmpA_.assign(static_cast<size_t>(maxBlock), 0.0f);
  tmpB_.assign(static_cast<size_t>(maxBlock), 0.0f);
  tmpC_.assign(static_cast<size_t>(maxBlock), 0.0f);
  tmpD_.assign(static_cast<size_t>(maxBlock), 0.0f);
}

// ------------------------------------------------------------------- cables

void Engine::setCables(const std::vector<Cable>& cables) {
  Routing r;
  for (const auto& c : cables) {
    if (c.in < 0 || c.in >= NUM_IN || c.out < 0 || c.out >= NUM_OUT) continue;
    int& n = r.count[c.in];
    if (n < kMaxSources) r.src[c.in][n++] = c.out;
  }
  std::lock_guard<std::mutex> lock(routingMutex_);
  pending_ = r;
  routingVersion_.fetch_add(1);
}

bool Engine::patched(int in) const {
  for (int i = 0; i < routing_.count[in]; ++i)
    if (moduleActive_[outInfo(routing_.src[in][i]).module]) return true;
  return false;
}

bool Engine::gather(int in, float* dst) const {
  bool any = false;
  std::fill(dst, dst + kB, 0.0f);
  for (int i = 0; i < routing_.count[in]; ++i) {
    const int o = routing_.src[in][i];
    if (!moduleActive_[outInfo(o).module]) continue;
    any = true;
    for (int k = 0; k < kB; ++k) dst[k] += outBuf_[o][k];
  }
  return any;
}

float Engine::cvAvg(int in) const {
  float buf[kB];
  if (!gather(in, buf)) return 0.0f;
  float s = 0.0f;
  for (int k = 0; k < kB; ++k) s += buf[k];
  return s / static_cast<float>(kB);
}

// --------------------------------------------------------------------- MIDI

void Engine::setLearned(int mask, int root) {
  learnedMask_ = mask & 0xfff;
  learnedRoot_ = root;
  scaleDirty_ = true;
  status_.midiMask.store(learnedMask_);
  status_.midiRoot.store(learnedRoot_);
}

void Engine::noteOn(int note) {
  if (note < 0 || note > 127) return;
  if (heldCount_ == 0) {
    learnedMask_ = 0;
    learnedRoot_ = note % 12;
  }
  if (!held_[static_cast<size_t>(note)]) {
    held_[static_cast<size_t>(note)] = true;
    ++heldCount_;
  }
  int lowest = 127;
  for (int i = 0; i < 128; ++i)
    if (held_[static_cast<size_t>(i)]) { lowest = i; break; }
  learnedRoot_ = lowest % 12;
  learnedMask_ |= 1 << (note % 12);
  scaleDirty_ = true;
  status_.midiMask.store(learnedMask_);
  status_.midiRoot.store(learnedRoot_);
  midiNote_ = note;
  midiStrum_ = true;
  midiTrig_ = true;
}

void Engine::noteOff(int note) {
  if (note < 0 || note > 127) return;
  if (held_[static_cast<size_t>(note)]) {
    held_[static_cast<size_t>(note)] = false;
    --heldCount_;
  }
}

void Engine::allNotesOff() {
  held_.fill(false);
  heldCount_ = 0;
}

void Engine::updateScale(const EngineParams& p) {
  auto& m = *impl_;
  const bool midi = p.scale >= scales::midiIndex();
  const int key = midi ? 100000 + learnedMask_ * 16 + (learnedRoot_ + 1) : p.scale;
  if (!scaleDirty_ && key == loadedScaleKey_) return;
  marbles::Scale s;
  s.base_interval = 1.0f;
  s.num_degrees = 0;
  if (midi) {
    if (learnedMask_ != 0 && learnedRoot_ >= 0) {
      for (int i = 0; i < 12; ++i) {
        const int pc = (learnedRoot_ + i) % 12;
        if (!(learnedMask_ & (1 << pc))) continue;
        s.degree[s.num_degrees].voltage = static_cast<float>(i) / 12.0f;
        s.degree[s.num_degrees].weight = static_cast<uint8_t>(scales::autoWeight(static_cast<float>(i)));
        ++s.num_degrees;
      }
    } else {
      s = kPresetScales[0];
    }
  } else {
    const auto& d = scales::all()[static_cast<size_t>(std::max(0, p.scale))];
    for (size_t k = 0; k < d.semis.size() && k < 16; ++k) {
      s.degree[k].voltage = d.semis[k] / 12.0f;
      s.degree[k].weight = static_cast<uint8_t>(d.weights.empty() ? scales::autoWeight(d.semis[k]) : d.weights[k]);
    }
    s.num_degrees = static_cast<int>(std::min<size_t>(d.semis.size(), 16));
  }
  m.xyGen.LoadScale(0, s);
  loadedScaleKey_ = key;
  scaleDirty_ = false;
}

void Engine::setHostTime(bool valid, bool playing, double ppq, double bpm) {
  hostValid_ = valid;
  hostPlaying_ = playing;
  hostPpq_ = ppq;
  hostBpm_ = bpm > 1.0 ? bpm : 120.0;
  rendered48_ = 0;
}

// ------------------------------------------------------------ bloque 48 kHz

void Engine::renderBlock(const EngineParams& p) {
  auto& m = *impl_;

  if (routingVersion_.load() != routingSeen_) {
    std::unique_lock<std::mutex> lock(routingMutex_, std::try_to_lock);
    if (lock.owns_lock()) {
      routing_ = pending_;
      routingSeen_ = routingVersion_.load();
    }
  }

  const bool gen = p.genOn;
  moduleActive_[MOD_IO] = moduleActive_[MOD_RINGS] = moduleActive_[MOD_CLOUDS] = true;
  moduleActive_[MOD_MARBLES] = moduleActive_[MOD_PLAITS] = gen;
  const float* A = p.att;
  auto mod01 = [&](float knob, int in) { return clamp01(knob + A[in] * cvAvg(in) * 0.2f); };

  // ------------------------------------------------------------- in / out
  float inL[kB], inR[kB];
  {
    float* ch[2] = {inL, inR};
    in48_.pop(ch, kB);
  }
  {
    const float att = 1.0f - std::exp(-1.0f / (0.005f * 48000.0f));
    const float rel = 1.0f - std::exp(-1.0f / (0.20f * 48000.0f));
    for (int k = 0; k < kB; ++k) {
      const float a = 0.5f * (std::abs(inL[k]) + std::abs(inR[k]));
      m.env += (a - m.env) * (a > m.env ? att : rel);
    }
    const float prev = status_.trackLevel.load(std::memory_order_relaxed);
    status_.trackLevel.store(std::max(prev, m.env * 2.0f), std::memory_order_relaxed);
  }

  // ------------------------------------------------------- random sampler
  if (gen) {
    updateScale(p);
    marbles::Ramps ramps;
    ramps.master = &m.rampBuffer[0];
    ramps.external = &m.rampBuffer[kB];
    ramps.slave[0] = &m.rampBuffer[kB * 2];
    ramps.slave[1] = &m.rampBuffer[kB * 3];

    float dv = mod01(p.dejaVu, IN_M_DV);
    if (dv < 0.47f) dv *= 1.06382978723f;
    else if (dv > 0.53f) dv = 0.5f + (dv - 0.53f) * 1.06382978723f;
    else dv = 0.5f;
    const int length = m.dvLengthQ.Lookup(kLoopLength, mod01(p.dvLength, IN_M_LEN));

    m.tGen.set_model(marbles::TGeneratorModel(p.tModel));
    m.tGen.set_range(marbles::TGeneratorRange(p.tRange));
    m.tGen.set_rate(mod01(p.tRate, IN_M_RATE) * 120.0f - 60.0f);
    m.tGen.set_bias(mod01(p.tBias, IN_M_BIAS));
    m.tGen.set_jitter(mod01(p.tJitter, IN_M_JITTER));
    m.tGen.set_deja_vu(p.dvT ? dv : 0.0f);
    m.tGen.set_length(length);
    m.tGen.set_pulse_width_mean(0.5f);
    m.tGen.set_pulse_width_std(0.0f);
    // Reloj: entrada T CLK (cable) o tempo del host.
    stmlib::GateFlags clock[kB];
    float clk[kB];
    bool external = gather(IN_M_TCLK, clk);
    if (external) {
      for (int k = 0; k < kB; ++k) {
        clock[k] = stmlib::ExtractGateFlags(clockPrev_, clk[k] > 1.0f);
        clockPrev_ = clock[k];
      }
    } else if (p.sync && hostValid_) {
      static const double perQuarter[8] = {0.25, 0.5, 1.0, 2.0, 4.0, 1.5, 3.0, 6.0};
      const double mult = perQuarter[std::min(std::max(p.syncDiv, 0), 7)];
      const double qps = hostBpm_ / 60.0 / 48000.0;
      for (int k = 0; k < kB; ++k) {
        const double ppq = hostPpq_ + static_cast<double>(rendered48_ + k) * qps;
        const double ph = ppq * mult;
        const bool high = hostPlaying_ && (ph - std::floor(ph)) < 0.5;
        clock[k] = stmlib::ExtractGateFlags(clockPrev_, high);
        clockPrev_ = clock[k];
      }
      external = true;
    }
    bool reset = false;
    m.tGen.Process(external, &reset, external ? clock : m.noClock, ramps, m.gates, kB);

    marbles::GroupSettings x, y;
    x.control_mode = marbles::CONTROL_MODE_IDENTICAL;
    x.voltage_range = marbles::VoltageRange(p.xRange);
    x.register_mode = false;
    x.register_value = 0.0f;
    x.spread = mod01(p.xSpread, IN_M_SPREAD);
    x.bias = mod01(p.xBias, IN_M_XBIAS);
    x.steps = mod01(p.xSteps, IN_M_STEPS);
    x.deja_vu = p.dvX ? dv : 0.0f;
    x.length = length;
    x.ratio.p = 1;
    x.ratio.q = 1;
    x.scale_index = 0;

    y.control_mode = marbles::CONTROL_MODE_IDENTICAL;
    y.voltage_range = marbles::VOLTAGE_RANGE_POSITIVE;
    y.register_mode = false;
    y.register_value = 0.0f;
    y.spread = 0.5f;
    y.bias = 0.5f;
    y.steps = 0.0f;
    y.deja_vu = 0.0f;
    y.length = 1;
    y.ratio.p = 1;
    y.ratio.q = 4;
    y.scale_index = x.scale_index;

    bool xReset = false;
    m.xyGen.Process(marbles::CLOCK_SOURCE_INTERNAL_T1_T2_T3, x, y, &xReset,
                    m.noClock, ramps, m.voltages, kB);

    // La tónica se aplica transponiendo las salidas X (el cuantizador trabaja en Do).
    const int root = (p.scale >= scales::midiIndex() && learnedRoot_ >= 0) ? learnedRoot_ : p.root;
    const float rootV = static_cast<float>(root) / 12.0f;
    for (int k = 0; k < kB; ++k) {
      outBuf_[OUT_M_T1][k] = m.gates[k * 2] ? 5.0f : 0.0f;
      outBuf_[OUT_M_T2][k] = ramps.master[k] < 0.5f ? 5.0f : 0.0f;
      outBuf_[OUT_M_T3][k] = m.gates[k * 2 + 1] ? 5.0f : 0.0f;
      outBuf_[OUT_M_X1][k] = m.voltages[k * 4 + 0] + rootV;
      outBuf_[OUT_M_X2][k] = m.voltages[k * 4 + 1] + rootV;
      outBuf_[OUT_M_X3][k] = m.voltages[k * 4 + 2] + rootV;
      outBuf_[OUT_M_Y][k] = m.voltages[k * 4 + 3];
    }
  } else {
    for (int o = OUT_M_T1; o <= OUT_M_X3; ++o) std::fill(outBuf_[o], outBuf_[o] + kB, 0.0f);
  }

  // ------------------------------------------------------ macro oscillator
  if (gen) {
    float voct[kB], trig[kB];
    const bool voctP = gather(IN_P_VOCT, voct);
    const bool trigP = gather(IN_P_TRIG, trig);
    const float fm = A[IN_P_FM] * cvAvg(IN_P_FM) * 12.0f;
    const float level = mod01(p.pLevel, IN_P_LEVEL);
    int engine = p.engine + static_cast<int>(std::lround(A[IN_P_MODEL] * cvAvg(IN_P_MODEL) * 0.2f * 24.0f));
    engine = std::min(std::max(engine, 0), 23);

    plaits::Patch patch;
    patch.harmonics = mod01(p.harmonics, IN_P_HARM);
    patch.timbre = mod01(p.timbre, IN_P_TIMBRE);
    patch.morph = mod01(p.morph, IN_P_MORPH);
    patch.frequency_modulation_amount = 0.0f;
    patch.timbre_modulation_amount = 0.0f;
    patch.morph_modulation_amount = 0.0f;
    patch.engine = engine;
    patch.decay = mod01(p.decay, IN_P_DECAY);
    patch.lpg_colour = mod01(p.lpgColour, IN_P_LPG);

    for (int b = 0; b < 2; ++b) {
      const int o = b * kPlaitsBlock;
      if (voctP) patch.note = 48.0f + p.pPitch + 12.0f * voct[o];
      else if (midiNote_ >= 0) patch.note = static_cast<float>(midiNote_) + p.pPitch;
      else patch.note = 48.0f + p.pPitch;
      patch.note = quantize(patch.note + fm, p.pQuant, p);

      plaits::Modulations mod;
      std::memset(&mod, 0, sizeof(mod));
      if (trigP) {
        float mx = 0.0f;
        for (int k = 0; k < kPlaitsBlock; ++k) mx = std::max(mx, trig[o + k]);
        mod.trigger = mx * 0.2f;
        mod.trigger_patched = true;
      } else if (midiNote_ >= 0) {
        mod.trigger_patched = true;
        if (midiTrig_) {
          mod.trigger = 0.0f;   // flanco limpio para la nota nueva
          midiTrig_ = false;
        } else {
          mod.trigger = heldCount_ > 0 ? 1.0f : 0.0f;
        }
      }

      plaits::Voice::Frame frames[kPlaitsBlock];
      m.plaits.Render(patch, mod, frames, kPlaitsBlock);
      for (int k = 0; k < kPlaitsBlock; ++k) {
        outBuf_[OUT_P_OUT][o + k] = static_cast<float>(frames[k].out) / 32768.0f * level * 5.0f;
        outBuf_[OUT_P_AUX][o + k] = static_cast<float>(frames[k].aux) / 32768.0f * level * 5.0f;
      }
    }
  } else {
    std::fill(outBuf_[OUT_P_OUT], outBuf_[OUT_P_OUT] + kB, 0.0f);
    std::fill(outBuf_[OUT_P_AUX], outBuf_[OUT_P_AUX] + kB, 0.0f);
  }

  // ------------------------------------------------------------- resonator
  {
    float exc[kB], out[kB], aux[kB], tmp[kB];
    // Excitador: audio de la pista (si EXCITER = TRACK) + cables a IN.
    const bool internalExciter = p.exciter == EXCITER_INTERNAL;
    const bool inPatched = gather(IN_R_IN, tmp);
    const float inLevel = A[IN_R_IN];  // atenuador de la entrada IN
    for (int k = 0; k < kB; ++k)
      exc[k] = ((p.exciter == EXCITER_TRACK ? 0.5f * (inL[k] + inR[k]) : 0.0f) +
                (inPatched ? tmp[k] * 0.2f : 0.0f)) * inLevel;
    // Puerta de ruido (rings.cc)
    const float kNoiseGateThreshold = 0.00003f;
    for (int k = 0; k < kB; ++k) {
      const float s = exc[k];
      const float error = s * s - m.gateLevel;
      m.gateLevel += error * (error > 0.0f ? 0.1f : 0.0001f);
      const float g = m.gateLevel <= kNoiseGateThreshold ? (1.0f / kNoiseGateThreshold) * m.gateLevel : 1.0f;
      exc[k] = g * s;
    }

    if (p.ringsOn) {
      rings::PerformanceState ps;
      rings::Patch rp;
      const float structure = mod01(p.structure, IN_R_STRUCT);
      rp.structure = std::min(structure, 0.9995f);
      rp.brightness = mod01(p.brightness, IN_R_BRIGHT);
      rp.damping = mod01(p.damping, IN_R_DAMP);
      rp.position = mod01(p.position, IN_R_POS);

      const float chord = structure * static_cast<float>(rings::kNumChords - 1);
      const float hyst = chord - m.chord > 0.0f ? -0.1f : 0.1f;
      m.chord = std::min(std::max(static_cast<int>(chord + hyst + 0.5f), 0), rings::kNumChords - 1);
      ps.chord = m.chord;
      ps.fm = std::min(std::max(A[IN_R_FM] * cvAvg(IN_R_FM) * 12.0f, -48.0f), 48.0f);
      ps.internal_exciter = internalExciter;

      float voct[kB], strum[kB];
      const bool voctP = gather(IN_R_VOCT, voct);
      const bool strumP = gather(IN_R_STRUM, strum);
      bool strumEdge = false;
      if (strumP) {
        for (int k = 0; k < kB; ++k) {
          if (strum[k] > 1.0f && m.prevStrum <= 1.0f) strumEdge = true;
          m.prevStrum = strum[k];
        }
      }

      if (voctP) {
        ps.internal_note = false;
        ps.tonic = 12.0f + std::round(p.ringsFreq);
        ps.note = voct[0] * 12.0f;
      } else if (midiNote_ >= 0) {
        ps.internal_note = false;
        ps.tonic = 12.0f;
        ps.note = static_cast<float>(midiNote_ - 12) + (std::round(p.ringsFreq) - 30.0f);
      } else {
        ps.internal_note = true;
        ps.tonic = 12.0f + p.ringsFreq;
        ps.note = 0.0f;
      }

      if (p.rQuant > 0) {
        if (ps.internal_note) ps.tonic = quantize(ps.tonic, p.rQuant, p);
        else ps.note = quantize(ps.tonic + ps.note, p.rQuant, p) - ps.tonic;
      }

      if (strumP) {
        ps.internal_strum = false;
        ps.strum = strumEdge;
      } else if (midiStrum_) {
        ps.internal_strum = false;
        ps.strum = true;
      } else {
        ps.internal_strum = true;
        ps.strum = false;
      }
      midiStrum_ = false;

      m.strummer.Process(internalExciter ? nullptr : exc, kB, &ps);
      // set_polyphony() reinicia los resonadores: solo al cambiar (como el botón).
      if (p.ringsPoly != m.part.polyphony()) m.part.set_polyphony(p.ringsPoly);
      m.part.set_model(rings::ResonatorModel(std::min(std::max(p.ringsModel, 0), 5)));
      m.part.Process(ps, rp, exc, out, aux, kB);
      if (ps.strum) status_.strum.store(1.0f);
    } else {
      midiStrum_ = false;
      std::copy(exc, exc + kB, out);
      std::copy(exc, exc + kB, aux);
    }
    for (int k = 0; k < kB; ++k) {
      outBuf_[OUT_R_ODD][k] = out[k] * 5.0f;
      outBuf_[OUT_R_EVEN][k] = aux[k] * 5.0f;
    }
  }

  // --------------------------------------------------------------- texture
  {
    float l[kB], r[kB];
    renderTexture(p, l, r);
    for (int k = 0; k < kB; ++k) {
      outBuf_[OUT_C_OUT_L][k] = l[k] * 5.0f;
      outBuf_[OUT_C_OUT_R][k] = r[k] * 5.0f;
    }
  }

  // ----------------------------------------------------------- salida final
  {
    float l[kB], r[kB];
    const bool lp = gather(IN_IO_OUT_L, l);
    const bool rp = gather(IN_IO_OUT_R, r);
    if (!lp && !rp) {
      std::copy(outBuf_[OUT_C_OUT_L], outBuf_[OUT_C_OUT_L] + kB, l);
      std::copy(outBuf_[OUT_C_OUT_R], outBuf_[OUT_C_OUT_R] + kB, r);
    } else if (lp && !rp) {
      std::copy(l, l + kB, r);
    } else if (!lp && rp) {
      std::copy(r, r + kB, l);
    }
    // +6 dB de compensación de nivel de la cadena (V → digital ×0.2 ×2).
    for (int k = 0; k < kB; ++k) { l[k] *= 0.4f * A[IN_IO_OUT_L]; r[k] *= 0.4f * A[IN_IO_OUT_R]; }
    m.mixEff = mod01(p.mix, IN_IO_MIX);
    const float* ch[2] = {l, r};
    rsOut_.process(ch, kB, outHost_);
  }

  rendered48_ += kB;

  // ----------------------------------------------------- estado para la GUI
  for (int o = 0; o < NUM_OUT; ++o) {
    float mx = 0.0f;
    for (int k = 0; k < kB; ++k) mx = std::max(mx, std::abs(outBuf_[o][k]));
    const float prev = status_.outLevel[o].load(std::memory_order_relaxed);
    status_.outLevel[o].store(std::max(mx * 0.2f, prev), std::memory_order_relaxed);
  }
}

// ----------------------------------------------------------------- texture
// 6 modos de Parasites (GranularProcessor) + TRIG DRUMS + DATA BENDER, a 32 kHz.
void Engine::renderTexture(const EngineParams& p, float* outL, float* outR) {
  auto& m = *impl_;
  const float* A = p.att;
  auto mod01 = [&](float knob, int in) { return clamp01(knob + A[in] * cvAvg(in) * 0.2f); };
  const int mode = std::min(std::max(p.cMode, 0), static_cast<int>(TEX_LAST) - 1);

  // Entradas (normalizadas a ODD/EVEN del resonador, cruzadas 60/40)
  float l[kB], r[kB];
  const bool lp = gather(IN_C_IN_L, l);
  const bool rp = gather(IN_C_IN_R, r);
  if (!lp && !rp) {
    for (int k = 0; k < kB; ++k) {
      const float o = outBuf_[OUT_R_ODD][k], e = outBuf_[OUT_R_EVEN][k];
      l[k] = 0.6f * o + 0.4f * e;
      r[k] = 0.6f * e + 0.4f * o;
    }
  } else if (lp && !rp) {
    std::copy(l, l + kB, r);
  } else if (!lp && rp) {
    std::copy(r, r + kB, l);
  }
  // 0.2: voltios -> digital; atenuadores de IN L / IN R
  for (int k = 0; k < kB; ++k) { l[k] *= 0.2f * A[IN_C_IN_L]; r[k] *= 0.2f * A[IN_C_IN_R]; }

  // Al entrar en DRUMS / BENDER: los knobs no saltan hasta que se muevan.
  const float knobNow[6] = {p.cPosition, p.cSize, (p.cPitch + 24.0f) / 48.0f,
                            p.cDensity, p.cTexture, p.cDryWet};
  if (mode != prevTexMode_) {
    if (mode == TEX_DRUMS) {
      for (int k = 0; k < 6; ++k) lastKnob_[k] = knobNow[k];
      drumPrmLoaded_ = false;
      for (auto& v : prevTrigV_) v = 0.0f;
    }
    if (mode < TEX_DRUMS && (ownsDrums_ || ownsBender_)) {
      texmodes::release(this);
      ownsDrums_ = ownsBender_ = false;
    }
    prevTexMode_ = mode;
  }

  // Puertas y disparos
  float buf[kB];
  bool freezeGate = false;
  if (mode == TEX_DRUMS) {
    // Cada jack de CV dispara una voz (como en el firmware): BD SD OH CH CP MT SA SB
    static const int jacks[8] = {IN_C_POS, IN_C_SIZE, IN_C_PITCH, IN_C_TRIG,
                                 IN_C_DENS, IN_C_TEX, IN_C_DRYWET, IN_C_FREEZE};
    for (int j = 0; j < 8; ++j) {
      if (!gather(jacks[j], buf)) { prevTrigV_[j] = 0.0f; continue; }
      for (int k = 0; k < kB; ++k) {
        if (buf[k] > 1.0f && prevTrigV_[j] <= 1.0f) {
          drumVel_[j] = std::max(drumVel_[j], std::min(std::max(buf[k] * 0.2f, 0.3f), 1.0f));
          if (j == 3) hatEdge_ = true;
        }
        prevTrigV_[j] = buf[k];
      }
    }
  } else {
    freezeGate = gather(IN_C_FREEZE, buf) && buf[kB - 1] > 1.0f;
    if (gather(IN_C_TRIG, buf)) {
      for (int k = 0; k < kB; ++k) {
        if (buf[k] > 1.0f && m.prevCTrig <= 1.0f) { m.cloudsTrig = true; benderClock_ = true; }
        m.prevCTrig = buf[k];
      }
    }
  }

  const float* ch[2] = {l, r};
  m.rs48to32.process(ch, kB, m.cIn32);
  while (m.cIn32.size() >= kCloudsBlock) {
    float bl[kCloudsBlock], br[kCloudsBlock];
    float* bc[2] = {bl, br};
    m.cIn32.pop(bc, kCloudsBlock);

    if (!p.cloudsOn) {
      // bypass: pasa la entrada
    } else if (mode < TEX_DRUMS) {
      clouds::ShortFrame input[kCloudsBlock], output[kCloudsBlock];
      for (int k = 0; k < kCloudsBlock; ++k) {
        input[k].l = toShort(bl[k]);
        input[k].r = toShort(br[k]);
      }
      m.clouds.set_playback_mode(clouds::PlaybackMode(mode));
      m.clouds.set_quality(std::min(std::max(p.cQuality, 0), 3));
      const bool wantReverse = mode == TEX_GRANULAR && (p.cAlt1 & 1);
      if (m.clouds.reversed() != wantReverse) m.clouds.ToggleReverse();
      clouds::Parameters* cp = m.clouds.mutable_parameters();
      cp->position = mod01(p.cPosition, IN_C_POS);
      cp->size = mod01(p.cSize, IN_C_SIZE);
      cp->pitch = std::min(std::max(p.cPitch + A[IN_C_PITCH] * cvAvg(IN_C_PITCH) * 12.0f, -48.0f), 48.0f);
      cp->density = mod01(p.cDensity, IN_C_DENS);
      cp->texture = mod01(p.cTexture, IN_C_TEX);
      cp->dry_wet = mod01(p.cDryWet, IN_C_DRYWET);
      cp->stereo_spread = mod01(p.cSpread, IN_C_SPREAD);
      cp->feedback = mod01(p.cFeedback, IN_C_FB);
      cp->reverb = mod01(p.cReverb, IN_C_REV);
      cp->freeze = p.cFreeze || freezeGate;
      cp->trigger = m.cloudsTrig;
      cp->gate = m.cloudsTrig;
      m.cloudsTrig = false;
      m.clouds.Process(input, output, kCloudsBlock);
      m.clouds.Prepare();
      for (int k = 0; k < kCloudsBlock; ++k) {
        bl[k] = static_cast<float>(output[k].l) / 32768.0f;
        br[k] = static_cast<float>(output[k].r) / 32768.0f;
      }
    } else if (mode == TEX_DRUMS) {
      if (!ownsDrums_) {
        ownsDrums_ = texmodes::acquire(texmodes::DRUMS, this);
        ownsBender_ = false;
        if (ownsDrums_) texmodes::drumsEnter();
      }
      status_.texBusy.store(ownsDrums_ ? 0 : 1);
      if (!ownsDrums_) {
        std::fill(bl, bl + kCloudsBlock, 0.0f);
        std::fill(br, br + kCloudsBlock, 0.0f);
      } else {
        if (!drumPrmLoaded_) { texmodes::drumsSetVoiceParams(drumPrm_); drumPrmLoaded_ = true; }
        texmodes::DrumControls dc;
        for (int j = 0; j < 8; ++j) { dc.vel[j] = drumVel_[j]; drumVel_[j] = 0.0f; }
        dc.clockEdge = hatEdge_;
        hatEdge_ = false;
        dc.voice = p.cAlt1 & 7;
        for (int k = 0; k < 6; ++k) {
          dc.knob[k] = knobNow[k];
          dc.moved[k] = std::abs(knobNow[k] - lastKnob_[k]) > 1.0e-4f;
          lastKnob_[k] = knobNow[k];
        }
        dc.kit = p.cAlt2 % 2;
        dc.audioIn = (p.cAlt2 / 2) % 3;
        dc.panWidth = p.cSpread;
        dc.echoFb = 0.75f * p.cFeedback;
        dc.rvTail = p.cReverb;
        dc.tempo = static_cast<float>(hostValid_ ? hostBpm_ : 120.0);
        dc.fx = p.cFx - 1;
        dc.dub = p.cFreeze;
        float ol[kCloudsBlock], orr[kCloudsBlock];
        texmodes::drumsProcess(dc, bl, br, ol, orr, kCloudsBlock);
        texmodes::drumsGetVoiceParams(drumPrm_);
        // La caja de ritmos sale a nivel de módulo (~0 dBFS): -10 dB para
        // igualarla con los demás modos de texture.
        for (int k = 0; k < kCloudsBlock; ++k) { bl[k] = ol[k] * 0.32f; br[k] = orr[k] * 0.32f; }
        for (int j = 0; j < 8; ++j) status_.drumHit[j].store(texmodes::drumsHit(j));
      }
    } else {  // TEX_BENDER
      if (!ownsBender_) {
        ownsBender_ = texmodes::acquire(texmodes::BENDER, this);
        ownsDrums_ = false;
        if (ownsBender_) texmodes::benderEnter();
      }
      status_.texBusy.store(ownsBender_ ? 0 : 1);
      if (!ownsBender_) {
        std::fill(bl, bl + kCloudsBlock, 0.0f);
        std::fill(br, br + kCloudsBlock, 0.0f);
      } else {
        texmodes::BenderControls bc2;
        bc2.time = mod01(p.cPosition, IN_C_POS);
        bc2.repeats = mod01(p.cSize, IN_C_SIZE);
        bc2.bend = clamp01((p.cPitch + 24.0f) / 48.0f);
        bc2.extraOct = cvAvg(IN_C_PITCH);  // 1 V/oct: velocidad MICRO
        bc2.brk = mod01(p.cDensity, IN_C_DENS);
        bc2.corrupt = mod01(p.cTexture, IN_C_TEX);
        bc2.mix = mod01(p.cDryWet, IN_C_DRYWET);
        bc2.width = mod01(p.cSpread, IN_C_SPREAD);
        bc2.regen = mod01(p.cFeedback, IN_C_FB);
        bc2.reverseChance = mod01(p.cReverb, IN_C_REV);
        bc2.mode = (p.cAlt1 % 6) % 3;
        bc2.reverse = (p.cAlt1 % 6) >= 3;
        bc2.corruptType = p.cAlt2 % 5;
        bc2.freeze = p.cFreeze || freezeGate;
        bc2.clockEdge = benderClock_;
        benderClock_ = false;
        bc2.fx = p.cFx - 1;
        float ol[kCloudsBlock], orr[kCloudsBlock];
        texmodes::benderProcess(bc2, bl, br, ol, orr, kCloudsBlock);
        std::copy(ol, ol + kCloudsBlock, bl);
        std::copy(orr, orr + kCloudsBlock, br);
        status_.sliceFlash.store(texmodes::benderSliceFlash());
      }
    }
    if (mode < TEX_DRUMS) status_.texBusy.store(0);
    const float* oc[2] = {bl, br};
    m.rs32to48.process(oc, kCloudsBlock, m.cOut48);
  }
  float* oc[2] = {outL, outR};
  m.cOut48.pop(oc, kB);
}

// ---------------------------------------------------------------- host

void Engine::process(const float* inL, const float* inR, float* outL, float* outR,
                     int n, const EngineParams& p) {
  if (static_cast<int>(tmpA_.size()) < n) {
    tmpA_.resize(static_cast<size_t>(n));
    tmpB_.resize(static_cast<size_t>(n));
    tmpC_.resize(static_cast<size_t>(n));
    tmpD_.resize(static_cast<size_t>(n));
  }
  if (inR == nullptr) inR = inL;
  for (int i = 0; i < n; ++i) {
    tmpA_[static_cast<size_t>(i)] = inL[i] * p.inputGain;
    tmpB_[static_cast<size_t>(i)] = inR[i] * p.inputGain;
  }
  const float* ch[2] = {tmpA_.data(), tmpB_.data()};
  rsIn_.process(ch, n, in48_);
  while (in48_.size() >= kB) renderBlock(p);

  const int dl = static_cast<int>(dryDelay_[0].size());
  float* wets[2] = {tmpC_.data(), tmpD_.data()};
  // Las entradas pueden compartir memoria con las salidas: leer la seca primero.
  for (int i = 0; i < n; ++i) {
    const size_t w = static_cast<size_t>(dryWrite_);
    const size_t r = static_cast<size_t>((dryWrite_ + 1) % dl);
    const float dL = dryDelay_[0][r], dR = dryDelay_[1][r];
    dryDelay_[0][w] = inL[i];
    dryDelay_[1][w] = inR[i];
    dryWrite_ = static_cast<int>(r);
    tmpA_[static_cast<size_t>(i)] = dL;
    tmpB_[static_cast<size_t>(i)] = dR;
  }
  if (outHost_.pop(wets, n) < n) ++underruns_;
  const float mix = impl_->mixEff;
  for (int i = 0; i < n; ++i) {
    const size_t k = static_cast<size_t>(i);
    outL[i] = (tmpA_[k] * (1.0f - mix) + tmpC_[k] * mix) * p.outputGain;
    outR[i] = (tmpB_[k] * (1.0f - mix) + tmpD_[k] * mix) * p.outputGain;
  }
}

}  // namespace ric
