// Modos extra de texture: TRIG DRUMS y DATA BENDER (firmware de mrarm84,
// MIT), más los efectos de actuación (perf FX) que comparten.
// Sus motores usan estado global: solo una instancia del plugin puede tener
// cada modo activo a la vez (ver acquire()).
#pragma once

#include <cstddef>

namespace texmodes {

enum Which { DRUMS = 0, BENDER = 1 };

// Reserva el motor para "owner". Devuelve false si otra instancia lo usa.
bool acquire(int which, const void* owner);
void release(const void* owner);

// ------------------------------------------------------------- TRIG DRUMS
// Voces en el orden de los jacks del módulo: BD SD OH CH CP MT SA SB.
struct DrumControls {
  float vel[8] = {};          // >0: golpe con esa velocidad (0..1)
  bool clockEdge = false;     // golpe de CH: también reloj del eco
  int voice = 0;              // voz que editan los knobs
  bool moved[6] = {};         // knob movido (salto al moverse)
  float knob[6] = {};         // POS, SIZE, PITCH, DENS, TEX, BLEND (0..1)
  int kit = 0;                // 0 ANALOG, 1 PUNCH
  int audioIn = 0;            // 0 THRU, 1 SKIN, 2 CHOP
  float panWidth = 0.33f, echoFb = 0.56f, rvTail = 0.67f;
  float tempo = 120.0f;       // tempo libre (sin reloj ni pulso detectado)
  int fx = -1;                // -1 = sin efecto, 0..7
  bool dub = false;
};
void drumsEnter();
void drumsProcess(const DrumControls& c, const float* inL, const float* inR,
                  float* outL, float* outR, size_t n);
float drumsHit(int jackVoice);          // destello del golpe (0..1)
void drumsGetVoiceParams(float* dst);   // 8 voces x 9 parámetros
void drumsSetVoiceParams(const float* src);
constexpr int kDrumParams = 8 * 9;
void drumsDefaultParams(float* dst);

// ------------------------------------------------------------ DATA BENDER
struct BenderControls {
  float time = 0.5f, repeats = 0.0f, bend = 0.5f, brk = 0.0f, corrupt = 0.0f, mix = 0.5f;
  float extraOct = 0.0f;      // V/OCT (octavas) para la velocidad MICRO
  int mode = 0;               // 0 TRAVERSE, 1 SILENCE, 2 MICRO
  int corruptType = 0;        // DECIMATE, DROPOUT, DESTROY, DJ FILTER, VINYL
  bool freeze = false, reverse = false, clockEdge = false;
  float width = 1.0f, regen = 0.0f, reverseChance = 0.0f;
  int fx = -1;
};
void benderEnter();
void benderProcess(const BenderControls& c, const float* inL, const float* inR,
                   float* outL, float* outR, size_t n);
float benderSliceFlash();

}  // namespace texmodes
