#include "../Source/Engine.h"
#include <cstdio>
#include <cmath>
#include <vector>
#include <random>
#include <complex>
using namespace ric; using namespace port;

struct Res { double rmsOn, rmsOff; bool nan; };
static std::mt19937 rng(3);
// Renderiza `secs` segundos; la entrada suena solo durante los primeros `inSecs`.
static std::vector<float> run(EngineParams p, std::vector<Cable> cables, double secs, double inSecs, int sr = 48000, int kind = 0) {
  Engine e; e.prepare(sr, 256); e.setCables(cables);
  std::vector<float> out; std::vector<float> in(256), l(256), r(256);
  std::uniform_real_distribution<float> u(-1, 1); double ph = 0;
  const int total = (int)(secs * sr);
  for (int pos = 0; pos < total; pos += 256) {
    for (int i = 0; i < 256; ++i) {
      const double t = (pos + i) / (double)sr;
      float s = 0;
      if (t < inSecs) { ph += 440.0 / sr; s = kind == 0 ? 0.3f * (float)sin(2 * M_PI * ph) : 0.2f * u(rng); }
      in[i] = s;
    }
    e.setHostTime(true, true, pos / (double)sr * 2.0, 120.0);
    e.process(in.data(), in.data(), l.data(), r.data(), 256, p);
    for (int i = 0; i < 256; ++i) out.push_back(0.5f * (l[i] + r[i]));
  }
  return out;
}
static double rms(const std::vector<float>& x, double a, double b, int sr = 48000) {
  double s = 0; int n = 0; for (int i = (int)(a * sr); i < (int)(b * sr) && i < (int)x.size(); ++i) { s += x[i] * x[i]; ++n; }
  return 20 * log10(sqrt(s / std::max(n, 1)) + 1e-9);
}
static bool hasNan(const std::vector<float>& x) { for (float v : x) if (!std::isfinite(v)) return true; return false; }
// frecuencia dominante en una ventana
static double peakHz(const std::vector<float>& x, double a, int sr = 48000) {
  const int N = 8192, s0 = (int)(a * sr); double best = 0, bf = 0;
  for (int k = 20; k < 1200; ++k) {  // hasta ~7 kHz
    std::complex<double> acc = 0;
    for (int n = 0; n < N; ++n) acc += (double)x[s0 + n] * (0.5 - 0.5 * cos(2 * M_PI * n / N)) * std::polar(1.0, -2 * M_PI * k * n / N);
    if (std::abs(acc) > best) { best = std::abs(acc); bf = k * (double)sr / N; }
  }
  return bf;
}

int main() {
  EngineParams base; base.ringsOn = false; base.mix = 1.0f; base.cDryWet = 1.0f; base.cReverb = 0.0f; base.cFeedback = 0.0f;

  // 1) FREEZE: sin cable vs con T2 -> FREEZE (GEN encendido, reloj lento)
  {
    EngineParams p = base; p.genOn = true; p.tRate = 0.3f; p.cMode = TEX_GRANULAR;
    p.att[IN_P_LEVEL] = 0; p.exciter = EXCITER_TRACK;
    auto a = run(p, {}, 6, 2, 48000, 1);
    auto b = run(p, {{OUT_M_T2, IN_C_FREEZE, 0}}, 6, 2, 48000, 1);
    printf("FREEZE  sin cable: %.1f dB tras cortar la entrada | con T2->FREEZE: %.1f dB\n", rms(a, 3, 6), rms(b, 3, 6));
  }
  // 2) TRIG: densidad 0.5 (sin granos propios) + T1 -> TRIG
  {
    EngineParams p = base; p.genOn = true; p.tRate = 0.6f; p.cMode = TEX_GRANULAR; p.cDensity = 0.5f;
    auto a = run(p, {}, 4, 4, 48000, 1);
    auto b = run(p, {{OUT_M_T1, IN_C_TRIG, 0}}, 4, 4, 48000, 1);
    printf("TRIG    densidad 12h sin cable: %.1f dB | con T1->TRIG: %.1f dB\n", rms(a, 1, 4), rms(b, 1, 4));
  }
  // 3) STRETCH
  {
    EngineParams p = base; p.cMode = TEX_STRETCH; p.cPitch = 12.0f;
    auto a = run(p, {}, 3, 3);
    printf("STRETCH pitch +12 sobre 440 Hz -> pico en %.0f Hz (esperado ~880)\n", peakHz(a, 2.0));
    p.cPitch = 0; p.cFreeze = false;
    EngineParams f = p; f.cFreeze = true;
    // congelado tras 1 s: la entrada sigue 1 s más; el contenido congelado debe sobrevivir sin entrada
    auto b = run(p, {}, 4, 1.5);
    printf("STRETCH sin freeze, tras cortar entrada: %.1f dB\n", rms(b, 2.5, 4));
  }
  // 4) Todos los modos con entrada de ruido
  const char* names[8] = {"Granular", "Stretch", "Looping", "Spectral", "Oliverb", "Resonestor", "Drums", "Bender"};
  for (int mode = 0; mode < 8; ++mode) {
    EngineParams p = base; p.cMode = mode; p.cDryWet = 0.7f; p.cReverb = 0.3f;
    std::vector<Cable> c;
    if (mode == TEX_DRUMS || mode == TEX_RESONESTOR) { p.genOn = true; p.tRate = 0.6f; }
    if (mode == TEX_DRUMS) c = {{OUT_M_T1, IN_C_POS, 0}, {OUT_M_T3, IN_C_SIZE, 1}, {OUT_M_T2, IN_C_TRIG, 2}};
    if (mode == TEX_RESONESTOR) c = {{OUT_M_T2, IN_C_TRIG, 0}};
    auto a = run(p, c, 5, mode == TEX_DRUMS ? 0 : 5, 48000, 1);
    printf("modo %-10s rms %.1f dB  nan %d\n", names[mode], rms(a, 1, 5), (int)hasNan(a));
  }
}
