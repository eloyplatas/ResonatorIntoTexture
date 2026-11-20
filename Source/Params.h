#pragma once

#include <string>
#include <array>
#include "Ports.h"

// Identificadores de parámetros (orden = índice en la caché del procesador).
namespace pid {
enum Index {
  IN_GAIN, OUT_GAIN, MIX,
  R_ON, R_FREQ, R_STRUCT, R_BRIGHT, R_DAMP, R_POS, R_MODEL, R_POLY,
  C_ON, C_POS, C_SIZE, C_PITCH, C_DENS, C_TEX, C_DRYWET, C_SPREAD, C_FB, C_REV,
  C_FREEZE, C_MODE, C_QUALITY, C_ALT1, C_ALT2, C_FX,
  GEN_ON, R_EXCITER, R_QUANT, P_QUANT,
  M_RATE, M_BIAS, M_JITTER, M_TMODEL, M_TRANGE, M_DV, M_LEN, M_DVT, M_DVX,
  M_SPREAD, M_XBIAS, M_STEPS, M_XRANGE, M_SCALE, M_ROOT, M_SYNC, M_SYNC_DIV,
  P_ENGINE, P_PITCH, P_HARM, P_TIMBRE, P_MORPH, P_DECAY, P_LPG, P_LEVEL,
  ATT_BASE,
  NUM_ATT = 36,
  COUNT = ATT_BASE + NUM_ATT
};

// Entradas de CV con atenuversor (orden = índice ATT_BASE + k).
inline int attPort(int k) {
  static const int ports[NUM_ATT] = {
    port::IN_IO_MIX,
    port::IN_M_RATE, port::IN_M_BIAS, port::IN_M_JITTER, port::IN_M_DV, port::IN_M_LEN,
    port::IN_M_SPREAD, port::IN_M_XBIAS, port::IN_M_STEPS,
    port::IN_P_FM, port::IN_P_HARM, port::IN_P_TIMBRE, port::IN_P_MORPH, port::IN_P_DECAY,
    port::IN_P_LPG, port::IN_P_LEVEL, port::IN_P_MODEL,
    port::IN_R_FM, port::IN_R_STRUCT, port::IN_R_BRIGHT, port::IN_R_DAMP, port::IN_R_POS,
    port::IN_C_POS, port::IN_C_SIZE, port::IN_C_PITCH, port::IN_C_DENS, port::IN_C_TEX,
    port::IN_C_DRYWET, port::IN_C_SPREAD, port::IN_C_FB, port::IN_C_REV,
    // Atenuadores de audio (0..1, por defecto 1)
    port::IN_R_IN, port::IN_C_IN_L, port::IN_C_IN_R, port::IN_IO_OUT_L, port::IN_IO_OUT_R,
  };
  return ports[k];
}

// Entrada de audio: su atenuador va de 0 a 1 (nivel), no es bipolar.
inline bool isAudioInput(int in) {
  return in == port::IN_R_IN || in == port::IN_C_IN_L || in == port::IN_C_IN_R ||
         in == port::IN_IO_OUT_L || in == port::IN_IO_OUT_R;
}

// Índice de parámetro del atenuversor de una entrada, o -1.
inline int attParamForPort(int in) {
  for (int k = 0; k < NUM_ATT; ++k)
    if (attPort(k) == in) return ATT_BASE + k;
  return -1;
}

inline const char* id(int i) {
  static const char* ids[] = {
    "in_gain", "out_gain", "mix",
    "r_on", "r_freq", "r_struct", "r_bright", "r_damp", "r_pos", "r_model", "r_poly",
    "c_on", "c_pos", "c_size", "c_pitch", "c_dens", "c_tex", "c_drywet", "c_spread", "c_fb", "c_rev",
    "c_freeze", "c_mode", "c_quality", "c_alt1", "c_alt2", "c_fx",
    "gen_on", "r_exciter", "r_quant", "p_quant",
    "m_rate", "m_bias", "m_jitter", "m_tmodel", "m_trange", "m_dv", "m_len", "m_dvt", "m_dvx",
    "m_spread", "m_xbias", "m_steps", "m_xrange", "m_scale", "m_root", "m_sync", "m_sync_div",
    "p_engine", "p_pitch", "p_harm", "p_timbre", "p_morph", "p_decay", "p_lpg", "p_level",
  };
  if (i >= ATT_BASE) {
    static const std::array<std::string, NUM_ATT> attIds = [] {
      std::array<std::string, NUM_ATT> a;
      for (int k = 0; k < NUM_ATT; ++k) a[static_cast<size_t>(k)] = std::string("att_") + port::inInfo(attPort(k)).key;
      return a;
    }();
    return attIds[static_cast<size_t>(i - ATT_BASE)].c_str();
  }
  return ids[i];
}
}  // namespace pid
