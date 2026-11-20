// Puertos (jacks) del sistema modular. Todas las señales viajan en voltios:
// audio ±5 V (= ±1.0 digital), puertas 0/5 V, CV según el caso.
#pragma once

#include <string>

namespace port {

enum Module { MOD_IO = 0, MOD_MARBLES, MOD_PLAITS, MOD_RINGS, MOD_CLOUDS, MOD_COUNT };

// ------------------------------------------------------------------ entradas
enum In {
  // in/out
  IN_IO_MIX = 0, IN_IO_OUT_L, IN_IO_OUT_R,
  // random sampler
  IN_M_RATE, IN_M_BIAS, IN_M_JITTER, IN_M_DV, IN_M_LEN, IN_M_SPREAD, IN_M_XBIAS, IN_M_STEPS,
  IN_M_TCLK,
  // macro oscillator
  IN_P_FM, IN_P_HARM, IN_P_TIMBRE, IN_P_MORPH, IN_P_DECAY, IN_P_LPG, IN_P_LEVEL, IN_P_MODEL,
  IN_P_VOCT, IN_P_TRIG,
  // resonator
  IN_R_FM, IN_R_STRUCT, IN_R_BRIGHT, IN_R_DAMP, IN_R_POS, IN_R_VOCT, IN_R_STRUM, IN_R_IN,
  // texture
  IN_C_POS, IN_C_SIZE, IN_C_PITCH, IN_C_DENS, IN_C_TEX, IN_C_DRYWET, IN_C_SPREAD, IN_C_FB,
  IN_C_REV, IN_C_FREEZE, IN_C_TRIG, IN_C_IN_L, IN_C_IN_R,
  NUM_IN
};

// ------------------------------------------------------------------- salidas
enum Out {
  OUT_M_T1 = 0, OUT_M_T2, OUT_M_T3, OUT_M_Y, OUT_M_X1, OUT_M_X2, OUT_M_X3,
  OUT_P_OUT, OUT_P_AUX,
  OUT_R_ODD, OUT_R_EVEN,
  OUT_C_OUT_L, OUT_C_OUT_R,
  NUM_OUT
};

struct Info {
  const char* key;   // identificador estable (estado / patches)
  const char* label; // texto del panel
  Module module;
};

inline const Info& inInfo(int i) {
  static const Info t[NUM_IN] = {
    {"io_mix", "MIX", MOD_IO}, {"io_out_l", "L", MOD_IO}, {"io_out_r", "R", MOD_IO},
    {"m_rate", "RATE", MOD_MARBLES}, {"m_bias", "BIAS", MOD_MARBLES}, {"m_jitter", "JITTER", MOD_MARBLES},
    {"m_dv", "DEJA VU", MOD_MARBLES}, {"m_len", "LENGTH", MOD_MARBLES}, {"m_spread", "SPREAD", MOD_MARBLES},
    {"m_xbias", "BIAS", MOD_MARBLES}, {"m_steps", "STEPS", MOD_MARBLES},
    {"m_tclk", "T CLK", MOD_MARBLES},
    {"p_fm", "FM", MOD_PLAITS}, {"p_harm", "HARMO", MOD_PLAITS}, {"p_timbre", "TIMBRE", MOD_PLAITS},
    {"p_morph", "MORPH", MOD_PLAITS}, {"p_decay", "DECAY", MOD_PLAITS}, {"p_lpg", "LPG", MOD_PLAITS},
    {"p_level", "LEVEL", MOD_PLAITS}, {"p_model", "MODEL", MOD_PLAITS},
    {"p_voct", "V/OCT", MOD_PLAITS}, {"p_trig", "TRIG", MOD_PLAITS},
    {"r_fm", "FM", MOD_RINGS}, {"r_struct", "STRUC", MOD_RINGS}, {"r_bright", "BRIGHT", MOD_RINGS},
    {"r_damp", "DAMP", MOD_RINGS}, {"r_pos", "POS", MOD_RINGS}, {"r_voct", "V/OCT", MOD_RINGS},
    {"r_strum", "STRUM", MOD_RINGS}, {"r_in", "IN", MOD_RINGS},
    {"c_pos", "POS", MOD_CLOUDS}, {"c_size", "SIZE", MOD_CLOUDS}, {"c_pitch", "PITCH", MOD_CLOUDS},
    {"c_dens", "DENS", MOD_CLOUDS}, {"c_tex", "TEXT", MOD_CLOUDS}, {"c_drywet", "BLEND", MOD_CLOUDS},
    {"c_spread", "SPRD", MOD_CLOUDS}, {"c_fb", "FDBK", MOD_CLOUDS}, {"c_rev", "VERB", MOD_CLOUDS},
    {"c_freeze", "FREEZE", MOD_CLOUDS}, {"c_trig", "TRIG", MOD_CLOUDS},
    {"c_in_l", "IN L", MOD_CLOUDS}, {"c_in_r", "IN R", MOD_CLOUDS},
  };
  return t[i];
}

inline const Info& outInfo(int i) {
  static const Info t[NUM_OUT] = {
    {"m_t1", "T1", MOD_MARBLES}, {"m_t2", "T2", MOD_MARBLES}, {"m_t3", "T3", MOD_MARBLES},
    {"m_y", "Y", MOD_MARBLES}, {"m_x1", "X1", MOD_MARBLES}, {"m_x2", "X2", MOD_MARBLES},
    {"m_x3", "X3", MOD_MARBLES},
    {"p_out", "OUT", MOD_PLAITS}, {"p_aux", "AUX", MOD_PLAITS},
    {"r_odd", "ODD", MOD_RINGS}, {"r_even", "EVEN", MOD_RINGS},
    {"c_out_l", "OUT L", MOD_CLOUDS}, {"c_out_r", "OUT R", MOD_CLOUDS},
  };
  return t[i];
}

inline int findIn(const char* key) {
  for (int i = 0; i < NUM_IN; ++i)
    if (std::string(inInfo(i).key) == key) return i;
  return -1;
}
inline int findOut(const char* key) {
  for (int i = 0; i < NUM_OUT; ++i)
    if (std::string(outInfo(i).key) == key) return i;
  return -1;
}

struct Cable {
  int out = 0;   // índice Out
  int in = 0;    // índice In
  int colour = 0;
  bool operator==(const Cable& o) const { return out == o.out && in == o.in; }
};

}  // namespace port
