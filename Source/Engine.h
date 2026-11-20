// ResonatorIntoTexture — motor DSP modular.
// Todo corre en bloques de 24 muestras a 48 kHz (Rings/Plaits/Marbles nativos);
// Clouds vive en su propio subdominio a 32 kHz. Los módulos se conectan con
// cables entre puertos (Ports.h); sin cable, cada entrada usa su normalización.
#pragma once

#include <atomic>
#include <array>
#include <memory>
#include <vector>
#include <cstdint>
#include <mutex>

#include "Resampler.h"
#include "Ports.h"

namespace ric {

// Excitador del resonador (además de los cables a su entrada IN).
enum Exciter { EXCITER_TRACK = 0, EXCITER_CABLES, EXCITER_INTERNAL };

// Modos de texture: 6 de Parasites + 2 de mrarm84.
enum TexMode {
  TEX_GRANULAR = 0, TEX_STRETCH, TEX_LOOPING, TEX_SPECTRAL, TEX_OLIVERB, TEX_RESONESTOR,
  TEX_DRUMS, TEX_BENDER, TEX_LAST
};

struct EngineParams {
  float inputGain = 1.0f, outputGain = 1.0f, mix = 1.0f;

  bool ringsOn = true;
  float ringsFreq = 30.0f;
  float structure = 0.4f, brightness = 0.6f, damping = 0.6f, position = 0.4f;
  int ringsModel = 0;
  int ringsPoly = 1;

  bool cloudsOn = true;
  float cPosition = 0.3f, cSize = 0.6f, cPitch = 0.0f, cDensity = 0.65f, cTexture = 0.6f;
  float cDryWet = 0.6f, cSpread = 0.6f, cFeedback = 0.3f, cReverb = 0.5f;
  bool cFreeze = false;
  int cMode = 0, cQuality = 0;
  int cAlt1 = 0, cAlt2 = 0, cFx = 0;   // cFx: 0 = sin efecto, 1..8

  bool genOn = false;
  int exciter = EXCITER_TRACK;
  int rQuant = 0, pQuant = 0;           // 0 libre, 1 semitonos, 2 escala

  float tRate = 0.45f, tBias = 0.5f, tJitter = 0.15f;
  int tModel = 0, tRange = 1;
  float dejaVu = 0.25f, dvLength = 0.5f;
  bool dvT = true, dvX = true;
  float xSpread = 0.5f, xBias = 0.5f, xSteps = 0.7f;
  int xRange = 0, scale = 0, root = 0;
  bool sync = false;
  int syncDiv = 3;   // índice en la tabla de divisiones (1/8)

  int engine = 8;
  float pPitch = 0.0f, harmonics = 0.5f, timbre = 0.5f, morph = 0.5f;
  float decay = 0.5f, lpgColour = 0.5f, pLevel = 0.7f;

  // Atenuversores (-1..+1) por entrada de CV; nivel (0..1) en las de audio.
  float att[port::NUM_IN] = {};
  EngineParams() {
    att[port::IN_R_IN] = att[port::IN_C_IN_L] = att[port::IN_C_IN_R] = 1.0f;
    att[port::IN_IO_OUT_L] = att[port::IN_IO_OUT_R] = 1.0f;
  }
};

struct EngineStatus {
  std::atomic<float> outLevel[port::NUM_OUT];
  std::atomic<float> inLevel[port::NUM_IN];
  std::atomic<float> strum{0};
  std::atomic<float> trackLevel{0};
  std::atomic<int> midiMask{0};
  std::atomic<int> midiRoot{-1};
  std::atomic<int> texBusy{0};          // 1: otra instancia usa DRUMS/BENDER
  std::atomic<float> drumHit[8];
  std::atomic<float> sliceFlash{0};
  EngineStatus() {
    for (auto& a : outLevel) a.store(0.0f);
    for (auto& a : inLevel) a.store(0.0f);
    for (auto& a : drumHit) a.store(0.0f);
  }
};

class Engine {
 public:
  Engine();
  ~Engine();

  void prepare(double hostRate, int maxBlock);
  void process(const float* inL, const float* inR, float* outL, float* outR,
               int n, const EngineParams& p);

  // Tiempo del host para la sincronía (llamar antes de process).
  void setHostTime(bool valid, bool playing, double ppq, double bpm);

  void noteOn(int note);
  void noteOff(int note);
  void allNotesOff();

  // Hilo de mensajes: nueva lista de cables.
  void setCables(const std::vector<port::Cable>& cables);

  int latencySamples() const { return latency_; }
  int underruns() const { return underruns_; }
  EngineStatus& status() { return status_; }

  // Parámetros por voz de TRIG DRUMS (8 x 9), guardados con el proyecto.
  void getDrumParams(float* dst) const;
  void setDrumParams(const float* src);

  int learnedMask() const { return learnedMask_; }
  int learnedRoot() const { return learnedRoot_; }
  void setLearned(int mask, int root);

  static constexpr int kBlock = 24;
  static constexpr int kMaxSources = 8;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  struct Routing {
    int count[port::NUM_IN] = {};
    int src[port::NUM_IN][kMaxSources] = {};
  };

  void renderBlock(const EngineParams& p);
  void updateScale(const EngineParams& p);
  // Suma de las salidas conectadas a una entrada; devuelve si está conectada.
  bool gather(int in, float* dst) const;
  bool patched(int in) const;
  float cvAvg(int in) const;   // media del bloque (V), 0 si no conectada
  float quantize(float note, int mode, const EngineParams& p) const;
  void renderTexture(const EngineParams& p, float* l, float* r);

  double hostRate_ = 48000.0;
  int latency_ = 0;
  int underruns_ = 0;

  Fifo in48_, outHost_;
  StreamResampler rsIn_, rsOut_;
  std::vector<float> dryDelay_[2];
  int dryWrite_ = 0;
  std::vector<float> tmpA_, tmpB_, tmpC_, tmpD_;

  float outBuf_[port::NUM_OUT][kBlock] = {};
  bool moduleActive_[port::MOD_COUNT] = {};

  Routing routing_;
  Routing pending_;
  std::mutex routingMutex_;
  std::atomic<int> routingVersion_{0};
  int routingSeen_ = -1;

  std::array<bool, 128> held_{};
  int heldCount_ = 0;
  bool hostValid_ = false, hostPlaying_ = false;
  double hostPpq_ = 0.0, hostBpm_ = 120.0;
  int rendered48_ = 0;
  uint8_t clockPrev_ = 0;
  int midiNote_ = -1;
  bool midiStrum_ = false;
  bool midiTrig_ = false;
  int learnedMask_ = 0;
  int learnedRoot_ = -1;
  int loadedScaleKey_ = -999;
  bool scaleDirty_ = true;

  // texture: disparos y estado de los modos DRUMS / BENDER
  float drumPrm_[72] = {};
  bool drumPrmLoaded_ = false;
  float lastKnob_[6] = {-1, -1, -1, -1, -1, -1};
  float drumVel_[8] = {};
  float prevTrigV_[8] = {};
  bool hatEdge_ = false;
  bool benderClock_ = false;
  bool ownsDrums_ = false, ownsBender_ = false;
  int prevTexMode_ = -1;

  EngineStatus status_;
};

}  // namespace ric
