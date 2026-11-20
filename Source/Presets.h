// Patches de fábrica: valores de parámetros (unidades reales; en los selectores,
// el índice de la opción) y cables. El valor tras un cable fija el atenuversor
// de esa entrada (si lo tiene).
#pragma once

#include <vector>
#include <utility>
#include "Params.h"

namespace presets {

struct PCable {
  const char* out;
  const char* in;
  float att;  // valor del atenuversor de la entrada (se ignora si no tiene)
};

struct Preset {
  const char* name;
  std::vector<std::pair<int, float>> values;
  std::vector<PCable> cables;
};

// Cableado estándar del generador: T1 dispara el oscilador, X1 su nota,
// T3 rasguea el resonador, X2 su nota.
#define GEN_STD {"m_t1", "p_trig", 0}, {"m_x1", "p_voct", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0}

inline const std::vector<Preset>& all() {
  using namespace pid;
  static const std::vector<Preset> list = {
    // ======================================================= GENERATIVOS
    {"Lluvia de cristal",
     {{OUT_GAIN, 12.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 9}, {P_PITCH, 12}, {P_HARM, 0.3f}, {P_TIMBRE, 0.45f}, {P_MORPH, 0.5f}, {P_DECAY, 0.35f}, {P_LPG, 0.7f}, {P_LEVEL, 0.7f},
      {R_MODEL, 1}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.35f}, {R_BRIGHT, 0.7f}, {R_DAMP, 0.8f}, {R_POS, 0.3f},
      {C_MODE, 0}, {C_POS, 0.4f}, {C_SIZE, 0.45f}, {C_DENS, 0.6f}, {C_TEX, 0.65f}, {C_DRYWET, 0.5f}, {C_SPREAD, 0.8f}, {C_FB, 0.25f}, {C_REV, 0.65f},
      {M_RATE, 0.58f}, {M_JITTER, 0.35f}, {M_DV, 0.3f}, {M_SPREAD, 0.45f}, {M_XBIAS, 0.55f}, {M_STEPS, 0.82f}, {M_SCALE, 2}, {M_ROOT, 0}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_timbre", 0.35f}, {"m_y", "c_pos", 0.25f}}},

    {"Órgano de niebla",
     {{OUT_GAIN, 10.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 12}, {P_PITCH, 0}, {P_HARM, 0.55f}, {P_TIMBRE, 0.4f}, {P_MORPH, 0.6f}, {P_DECAY, 0.85f}, {P_LPG, 0.15f}, {P_LEVEL, 1.0f},
      {R_MODEL, 5}, {R_POLY, 2}, {R_FREQ, 24}, {R_STRUCT, 0.2f}, {R_BRIGHT, 0.5f}, {R_DAMP, 0.8f}, {R_POS, 0.5f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.8f}, {C_DENS, 0.5f}, {C_TEX, 0.7f}, {C_DRYWET, 0.6f}, {C_FB, 0.2f}, {C_REV, 0.7f},
      {M_RATE, 0.3f}, {M_JITTER, 0.2f}, {M_TMODEL, 1}, {M_DV, 0.5f}, {M_SPREAD, 0.3f}, {M_STEPS, 0.85f}, {M_SCALE, 0}, {M_ROOT, 2}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_morph", 0.4f}}},

    {"Kalimba aleatoria",
     {{OUT_GAIN, 9.0f}, {GEN_ON, 1}, {R_EXCITER, 2},
      {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 36}, {R_STRUCT, 0.25f}, {R_BRIGHT, 0.55f}, {R_DAMP, 0.55f}, {R_POS, 0.25f},
      {C_MODE, 0}, {C_POS, 0.3f}, {C_SIZE, 0.35f}, {C_DENS, 0.45f}, {C_TEX, 0.6f}, {C_DRYWET, 0.35f}, {C_REV, 0.35f},
      {M_RATE, 0.62f}, {M_JITTER, 0.15f}, {M_SPREAD, 0.55f}, {M_STEPS, 0.9f}, {M_SCALE, 0}, {M_ROOT, 0}},
     {{"m_t1", "r_strum", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0}, {"m_y", "r_bright", 0.3f}}},

    {"Arpa eólica",
     {{OUT_GAIN, 1.0f}, {GEN_ON, 1}, {R_EXCITER, 2},
      {R_MODEL, 1}, {R_POLY, 2}, {R_FREQ, 24}, {R_STRUCT, 0.55f}, {R_BRIGHT, 0.45f}, {R_DAMP, 0.8f}, {R_POS, 0.6f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.7f}, {C_DENS, 0.5f}, {C_TEX, 0.65f}, {C_DRYWET, 0.55f}, {C_REV, 0.65f},
      {M_RATE, 0.4f}, {M_TMODEL, 1}, {M_JITTER, 0.5f}, {M_DV, 0.25f}, {M_SPREAD, 0.6f}, {M_STEPS, 0.75f}, {M_SCALE, 4}, {M_ROOT, 4}},
     {{"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0}, {"m_y", "r_pos", 0.4f}, {"m_y", "r_struct", 0.2f}}},

    {"Pelog nocturno",
     {{OUT_GAIN, 12.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 10}, {P_HARM, 0.25f}, {P_TIMBRE, 0.35f}, {P_MORPH, 0.3f}, {P_DECAY, 0.5f}, {P_LPG, 0.5f}, {P_LEVEL, 0.7f},
      {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.45f}, {R_BRIGHT, 0.5f}, {R_DAMP, 0.7f}, {R_POS, 0.45f},
      {C_MODE, 0}, {C_POS, 0.3f}, {C_SIZE, 0.6f}, {C_DENS, 0.62f}, {C_TEX, 0.55f}, {C_DRYWET, 0.55f}, {C_SPREAD, 0.7f}, {C_FB, 0.3f}, {C_REV, 0.6f},
      {M_RATE, 0.45f}, {M_JITTER, 0.3f}, {M_SPREAD, 0.5f}, {M_STEPS, 0.8f}, {M_SCALE, 3}, {M_ROOT, 0}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_timbre", 0.3f}, {"m_y", "c_pos", 0.2f}}},

    {"Coro de partículas",
     {{OUT_GAIN, -16.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 18}, {P_HARM, 0.5f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.6f}, {P_DECAY, 0.6f}, {P_LPG, 0.3f}, {P_LEVEL, 0.8f},
      {R_MODEL, 2}, {R_POLY, 1}, {R_FREQ, 30}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.6f}, {R_DAMP, 0.75f}, {R_POS, 0.2f},
      {C_MODE, 3}, {C_POS, 0.5f}, {C_SIZE, 0.5f}, {C_DENS, 0.5f}, {C_TEX, 0.6f}, {C_DRYWET, 0.55f}, {C_REV, 0.55f},
      {M_RATE, 0.5f}, {M_TMODEL, 1}, {M_JITTER, 0.4f}, {M_DV, 0.4f}, {M_SPREAD, 0.5f}, {M_STEPS, 0.85f}, {M_SCALE, 1}, {M_ROOT, 9}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "c_tex", 0.3f}}},

    {"Voces en el pozo",
     {{OUT_GAIN, 9.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 15}, {P_HARM, 0.4f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.5f}, {P_DECAY, 0.55f}, {P_LPG, 0.4f}, {P_LEVEL, 0.8f},
      {R_MODEL, 0}, {R_POLY, 0}, {R_FREQ, 18}, {R_STRUCT, 0.6f}, {R_BRIGHT, 0.45f}, {R_DAMP, 0.78f}, {R_POS, 0.5f},
      {C_MODE, 0}, {C_POS, 0.6f}, {C_SIZE, 0.75f}, {C_DENS, 0.58f}, {C_TEX, 0.7f}, {C_DRYWET, 0.6f}, {C_REV, 0.7f},
      {M_RATE, 0.4f}, {M_DV, 0.6f}, {M_SPREAD, 0.4f}, {M_STEPS, 0.9f}, {M_SCALE, 1}, {M_ROOT, 2}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_morph", 0.6f}, {"m_x3", "p_harm", 0.4f}}},

    {"Enjambre",
     {{OUT_GAIN, 0.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 16}, {P_HARM, 0.4f}, {P_TIMBRE, 0.4f}, {P_MORPH, 0.5f}, {P_LEVEL, 0.45f},
      {R_MODEL, 4}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.4f}, {R_BRIGHT, 0.4f}, {R_DAMP, 0.85f}, {R_POS, 0.4f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.9f}, {C_DENS, 0.5f}, {C_TEX, 0.6f}, {C_DRYWET, 0.55f}, {C_REV, 0.6f},
      {M_RATE, 0.25f}, {M_SPREAD, 0.35f}, {M_STEPS, 0.95f}, {M_SCALE, 2}, {M_ROOT, 7}},
     {{"p_out", "r_in", 0}, {"m_x1", "p_voct", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0},
      {"m_y", "p_timbre", 0.5f}, {"m_y", "p_harm", 0.3f}}},

    {"Tambores de lata",
     {{OUT_GAIN, 12.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 21}, {P_PITCH, -6}, {P_HARM, 0.4f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.4f}, {P_DECAY, 0.4f}, {P_LEVEL, 0.8f},
      {R_MODEL, 3}, {R_POLY, 1}, {R_FREQ, 30}, {R_STRUCT, 0.35f}, {R_BRIGHT, 0.6f}, {R_DAMP, 0.6f}, {R_POS, 0.4f},
      {C_MODE, 2}, {C_POS, 0.5f}, {C_SIZE, 0.4f}, {C_DENS, 0.5f}, {C_TEX, 0.5f}, {C_DRYWET, 0.4f}, {C_FB, 0.45f}, {C_REV, 0.3f},
      {M_RATE, 0.6f}, {M_TMODEL, 2}, {M_JITTER, 0.1f}, {M_DV, 0.7f}, {M_LEN, 0.4f}, {M_SPREAD, 0.5f}, {M_STEPS, 0.8f}, {M_SCALE, 2}},
     {{"p_out", "r_in", 0}, {"m_t1", "p_trig", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0}, {"m_y", "c_pos", 0.3f}}},

    {"Cadena de Markov",
     {{OUT_GAIN, 5.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 13}, {P_HARM, 0.5f}, {P_TIMBRE, 0.4f}, {P_MORPH, 0.5f}, {P_DECAY, 0.45f}, {P_LPG, 0.6f}, {P_LEVEL, 0.7f},
      {R_MODEL, 2}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.2f}, {R_BRIGHT, 0.65f}, {R_DAMP, 0.82f}, {R_POS, 0.3f},
      {C_MODE, 0}, {C_POS, 0.35f}, {C_SIZE, 0.5f}, {C_DENS, 0.65f}, {C_TEX, 0.5f}, {C_DRYWET, 0.5f}, {C_REV, 0.62f}, {C_FB, 0.3f},
      {M_RATE, 0.55f}, {M_TMODEL, 6}, {M_SPREAD, 0.5f}, {M_STEPS, 0.8f}, {M_SCALE, 0}, {M_ROOT, 5}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_morph", 0.5f}, {"m_x3", "r_bright", 0.2f}}},

    {"Glaciar",
     {{OUT_GAIN, 5.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 14}, {P_PITCH, -12}, {P_HARM, 0.4f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.4f}, {P_DECAY, 0.9f}, {P_LPG, 0.1f}, {P_LEVEL, 0.6f},
      {R_MODEL, 5}, {R_POLY, 2}, {R_FREQ, 18}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.4f}, {R_DAMP, 0.9f}, {R_POS, 0.5f},
      {C_MODE, 1}, {C_PITCH, -12}, {C_POS, 0.5f}, {C_SIZE, 0.9f}, {C_DENS, 0.5f}, {C_TEX, 0.75f}, {C_DRYWET, 0.65f}, {C_FB, 0.35f}, {C_REV, 0.8f},
      {M_RATE, 0.5f}, {M_TRANGE, 0}, {M_SPREAD, 0.4f}, {M_STEPS, 0.85f}, {M_SCALE, 1}, {M_ROOT, 4}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "c_pos", 0.3f}, {"m_y", "p_harm", 0.2f}}},

    {"Campanas tibetanas",
     {{OUT_GAIN, 10.0f}, {GEN_ON, 1}, {R_EXCITER, 2},
      {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 12}, {R_STRUCT, 0.72f}, {R_BRIGHT, 0.35f}, {R_DAMP, 0.9f}, {R_POS, 0.35f},
      {C_MODE, 0}, {C_POS, 0.4f}, {C_SIZE, 0.8f}, {C_DENS, 0.5f}, {C_TEX, 0.7f}, {C_DRYWET, 0.5f}, {C_REV, 0.7f},
      {M_RATE, 0.3f}, {M_JITTER, 0.6f}, {M_SPREAD, 0.5f}, {M_STEPS, 0.7f}, {M_SCALE, 5}, {M_ROOT, 2}},
     {{"m_t1", "r_strum", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0}, {"m_y", "r_pos", 0.3f}}},

    {"Máquina de cuerdas",
     {{OUT_GAIN, 1.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 6}, {P_HARM, 0.5f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.4f}, {P_LEVEL, 0.45f},
      {R_MODEL, 1}, {R_POLY, 0}, {R_FREQ, 24}, {R_STRUCT, 0.5f}, {R_BRIGHT, 0.5f}, {R_DAMP, 0.9f}, {R_POS, 0.4f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.8f}, {C_DENS, 0.5f}, {C_TEX, 0.65f}, {C_DRYWET, 0.55f}, {C_REV, 0.7f},
      {M_RATE, 0.3f}, {M_TMODEL, 1}, {M_SPREAD, 0.3f}, {M_STEPS, 0.9f}, {M_SCALE, 0}, {M_ROOT, 0}},
     {{"p_out", "r_in", 0}, {"m_x1", "p_voct", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0},
      {"m_y", "p_timbre", 0.4f}, {"m_y", "c_pos", 0.2f}}},

    {"Chiptune en la nieve",
     {{OUT_GAIN, 4.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 7}, {P_PITCH, 12}, {P_HARM, 0.3f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.5f}, {P_DECAY, 0.3f}, {P_LPG, 0.8f}, {P_LEVEL, 0.6f},
      {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 36}, {R_STRUCT, 0.15f}, {R_BRIGHT, 0.7f}, {R_DAMP, 0.5f}, {R_POS, 0.2f},
      {C_MODE, 0}, {C_PITCH, 12}, {C_POS, 0.3f}, {C_SIZE, 0.3f}, {C_DENS, 0.7f}, {C_TEX, 0.4f}, {C_DRYWET, 0.45f}, {C_REV, 0.5f},
      {M_RATE, 0.68f}, {M_SPREAD, 0.55f}, {M_STEPS, 0.85f}, {M_SCALE, 2}, {M_ROOT, 0}},
     {{"p_out", "r_in", 0}, GEN_STD}},

    {"Seis operadores",
     {{OUT_GAIN, 3.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 2}, {P_HARM, 0.3f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.5f}, {P_DECAY, 0.6f}, {P_LPG, 0.4f}, {P_LEVEL, 0.7f},
      {R_MODEL, 0}, {R_POLY, 2}, {R_FREQ, 24}, {R_STRUCT, 0.4f}, {R_BRIGHT, 0.5f}, {R_DAMP, 0.85f}, {R_POS, 0.4f},
      {C_MODE, 0}, {C_POS, 0.4f}, {C_SIZE, 0.6f}, {C_DENS, 0.55f}, {C_TEX, 0.6f}, {C_DRYWET, 0.5f}, {C_REV, 0.7f}, {C_FB, 0.3f},
      {M_RATE, 0.45f}, {M_JITTER, 0.25f}, {M_SPREAD, 0.55f}, {M_STEPS, 0.8f}, {M_SCALE, 0}, {M_ROOT, 7}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_morph", 0.3f}, {"m_x3", "p_harm", 0.3f}}},

    {"Terreno ondulado",
     {{OUT_GAIN, 5.5f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 5}, {P_HARM, 0.5f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.5f}, {P_DECAY, 0.55f}, {P_LPG, 0.5f}, {P_LEVEL, 0.7f},
      {R_MODEL, 2}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.55f}, {R_DAMP, 0.7f}, {R_POS, 0.4f},
      {C_MODE, 3}, {C_POS, 0.4f}, {C_SIZE, 0.6f}, {C_DENS, 0.5f}, {C_TEX, 0.5f}, {C_DRYWET, 0.55f}, {C_REV, 0.55f},
      {M_RATE, 0.45f}, {M_JITTER, 0.35f}, {M_SPREAD, 0.5f}, {M_STEPS, 0.8f}, {M_SCALE, 1}, {M_ROOT, 0}},
     {{"p_out", "r_in", 0}, GEN_STD, {"m_y", "p_morph", 0.5f}, {"m_x3", "p_timbre", 0.3f}}},

    {"Ruido del mar",
     {{OUT_GAIN, 12.0f}, {GEN_ON, 1}, {R_EXCITER, 1},
      {P_ENGINE, 17}, {P_HARM, 0.3f}, {P_TIMBRE, 0.5f}, {P_MORPH, 0.7f}, {P_LEVEL, 0.5f},
      {R_MODEL, 5}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.25f}, {R_BRIGHT, 0.4f}, {R_DAMP, 0.8f}, {R_POS, 0.5f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.9f}, {C_DENS, 0.5f}, {C_TEX, 0.7f}, {C_DRYWET, 0.6f}, {C_REV, 0.7f},
      {M_RATE, 0.2f}, {M_SPREAD, 0.3f}, {M_STEPS, 0.9f}, {M_SCALE, 2}, {M_ROOT, 9}},
     {{"p_out", "r_in", 0}, {"m_x1", "p_voct", 0}, {"m_t3", "r_strum", 0}, {"m_x2", "r_voct", 0},
      {"m_y", "p_timbre", 0.6f}, {"m_y", "p_harm", 0.3f}, {"m_y", "c_size", 0.2f}}},

    // ========================================================== EFECTOS
    {"Campana sobre la voz",
     {{OUT_GAIN, 6.0f}, {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 30}, {R_STRUCT, 0.5f}, {R_BRIGHT, 0.55f}, {R_DAMP, 0.7f}, {R_POS, 0.35f},
      {C_MODE, 0}, {C_POS, 0.4f}, {C_SIZE, 0.7f}, {C_DENS, 0.55f}, {C_TEX, 0.7f}, {C_DRYWET, 0.55f}, {C_SPREAD, 0.7f}, {C_FB, 0.2f}, {C_REV, 0.55f}},
     {}},

    {"Cuerdas simpáticas",
     {{OUT_GAIN, 1.0f}, {R_MODEL, 1}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.6f}, {R_DAMP, 0.8f}, {R_POS, 0.4f},
      {C_MODE, 1}, {C_POS, 0.5f}, {C_SIZE, 0.7f}, {C_DENS, 0.5f}, {C_TEX, 0.6f}, {C_DRYWET, 0.5f}, {C_REV, 0.6f}, {MIX, 0.8f}},
     {}},

    {"Sitar eléctrico",
     {{OUT_GAIN, 1.5f}, {R_MODEL, 2}, {R_POLY, 0}, {R_FREQ, 28}, {R_STRUCT, 0.25f}, {R_BRIGHT, 0.8f}, {R_DAMP, 0.75f}, {R_POS, 0.15f},
      {C_MODE, 2}, {C_POS, 0.5f}, {C_SIZE, 0.55f}, {C_DENS, 0.5f}, {C_TEX, 0.5f}, {C_DRYWET, 0.45f}, {C_SPREAD, 0.7f}, {C_FB, 0.45f}, {C_REV, 0.4f}, {MIX, 0.85f}},
     {}},

    {"Espectro que respira",
     {{OUT_GAIN, 0.5f}, {R_MODEL, 0}, {R_POLY, 1}, {R_FREQ, 30}, {R_STRUCT, 0.4f}, {R_BRIGHT, 0.6f}, {R_DAMP, 0.7f}, {R_POS, 0.3f},
      {C_MODE, 3}, {C_POS, 0.5f}, {C_SIZE, 0.6f}, {C_DENS, 0.62f}, {C_TEX, 0.5f}, {C_DRYWET, 0.6f}, {C_REV, 0.5f}, {MIX, 0.9f}},
     {}},

    {"Eco de ruiseñor",
     {{OUT_GAIN, 5.5f}, {R_MODEL, 5}, {R_POLY, 1}, {R_FREQ, 36}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.7f}, {R_DAMP, 0.7f}, {R_POS, 0.5f},
      {C_MODE, 2}, {C_PITCH, 12}, {C_POS, 0.5f}, {C_SIZE, 0.5f}, {C_DENS, 0.5f}, {C_TEX, 0.5f}, {C_DRYWET, 0.45f}, {C_FB, 0.5f}, {C_REV, 0.4f}, {MIX, 0.8f}},
     {}},

    {"Gamelan de entrada",
     {{OUT_GAIN, 5.0f}, {R_MODEL, 3}, {R_POLY, 1}, {R_FREQ, 30}, {R_STRUCT, 0.45f}, {R_BRIGHT, 0.6f}, {R_DAMP, 0.7f}, {R_POS, 0.4f},
      {C_MODE, 0}, {C_PITCH, -12}, {C_POS, 0.4f}, {C_SIZE, 0.6f}, {C_DENS, 0.65f}, {C_TEX, 0.6f}, {C_DRYWET, 0.5f}, {C_REV, 0.55f}},
     {}},

    {"Bajo lejano",
     {{OUT_GAIN, 4.0f}, {R_MODEL, 0}, {R_POLY, 0}, {R_FREQ, 12}, {R_STRUCT, 0.3f}, {R_BRIGHT, 0.3f}, {R_DAMP, 0.85f}, {R_POS, 0.3f},
      {C_MODE, 1}, {C_PITCH, -12}, {C_POS, 0.5f}, {C_SIZE, 0.8f}, {C_DENS, 0.5f}, {C_TEX, 0.7f}, {C_DRYWET, 0.55f}, {C_REV, 0.7f}, {MIX, 0.8f}},
     {}},

    {"Respira",
     {{OUT_GAIN, 1.5f}, {R_MODEL, 4}, {R_POLY, 1}, {R_FREQ, 24}, {R_STRUCT, 0.45f}, {R_BRIGHT, 0.35f}, {R_DAMP, 0.75f}, {R_POS, 0.4f},
      {C_MODE, 0}, {C_POS, 0.45f}, {C_SIZE, 0.5f}, {C_DENS, 0.55f}, {C_TEX, 0.65f}, {C_DRYWET, 0.55f}, {C_REV, 0.6f}, {MIX, 0.9f}},
     {}},
  };
  return list;
}

#undef GEN_STD

}  // namespace presets
