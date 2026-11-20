// Lista de escalas de cuantización del random sampler.
// Los 6 primeros son las escalas de fábrica de Marbles (con sus pesos);
// el resto usa pesos automáticos (tónica > quinta > tercera > resto).
// Los grados pueden ser microtonales (semitonos con decimales).
#pragma once

#include <vector>
#include <cmath>

namespace scales {

struct Def {
  const char* name;
  const char* group;
  std::vector<float> semis;
  std::vector<int> weights;  // vacío = automáticos
};

inline const std::vector<Def>& all() {
  static const std::vector<Def> list = {
    // --- Marbles (pesos originales)
    {"Major", "Marbles", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {255, 16, 96, 24, 128, 64, 8, 192, 16, 96, 24, 128}},
    {"Minor", "Marbles", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {255, 16, 96, 128, 8, 64, 4, 192, 96, 16, 128, 16}},
    {"Pentatonic", "Marbles", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {255, 4, 96, 4, 4, 140, 4, 192, 4, 96, 4, 4}},
    {"Pelog", "Marbles", {0, 1.53f, 3.15f, 5.52f, 7.06f, 8.48f, 10.58f}, {255, 128, 32, 8, 192, 64, 16}},
    {"Raag Bhairav", "Marbles", {0, 0.902f, 2.039f, 3.156f, 3.863f, 4.98f, 5.902f, 7.02f, 7.921f, 9.059f, 10.175f, 10.883f},
     {255, 128, 4, 4, 128, 64, 4, 192, 64, 4, 4, 64}},
    {"Raag Shri", "Marbles", {0, 0.902f, 2.039f, 3.156f, 3.863f, 4.98f, 5.902f, 7.02f, 7.921f, 9.059f, 10.175f, 10.883f},
     {255, 4, 128, 64, 4, 128, 4, 192, 4, 64, 128, 4}},
    // --- Modos
    {"Ionian (mayor)", "Modos", {0, 2, 4, 5, 7, 9, 11}, {}},
    {"Dorian", "Modos", {0, 2, 3, 5, 7, 9, 10}, {}},
    {"Phrygian", "Modos", {0, 1, 3, 5, 7, 8, 10}, {}},
    {"Lydian", "Modos", {0, 2, 4, 6, 7, 9, 11}, {}},
    {"Mixolydian", "Modos", {0, 2, 4, 5, 7, 9, 10}, {}},
    {"Aeolian (menor nat.)", "Modos", {0, 2, 3, 5, 7, 8, 10}, {}},
    {"Locrian", "Modos", {0, 1, 3, 5, 6, 8, 10}, {}},
    {"Harmonic minor", "Modos", {0, 2, 3, 5, 7, 8, 11}, {}},
    {"Melodic minor", "Modos", {0, 2, 3, 5, 7, 9, 11}, {}},
    {"Lydian dominant", "Modos", {0, 2, 4, 6, 7, 9, 10}, {}},
    {"Phrygian dominant", "Modos", {0, 1, 4, 5, 7, 8, 10}, {}},
    // --- Pentatónicas y otras
    {"Major pentatonic", "Pentatonicas", {0, 2, 4, 7, 9}, {}},
    {"Minor pentatonic", "Pentatonicas", {0, 3, 5, 7, 10}, {}},
    {"Blues", "Pentatonicas", {0, 3, 5, 6, 7, 10}, {}},
    {"Hirajoshi", "Pentatonicas", {0, 2, 3, 7, 8}, {}},
    {"In sen", "Pentatonicas", {0, 1, 5, 7, 10}, {}},
    {"Iwato", "Pentatonicas", {0, 1, 5, 6, 10}, {}},
    {"Kumoi", "Pentatonicas", {0, 2, 3, 7, 9}, {}},
    {"Yo", "Pentatonicas", {0, 2, 5, 7, 9}, {}},
    // --- Simétricas
    {"Chromatic", "Simetricas", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {}},
    {"Whole tone", "Simetricas", {0, 2, 4, 6, 8, 10}, {}},
    {"Octatonic H-W", "Simetricas", {0, 1, 3, 4, 6, 7, 9, 10}, {}},
    {"Octatonic W-H", "Simetricas", {0, 2, 3, 5, 6, 8, 9, 11}, {}},
    {"Augmented", "Simetricas", {0, 3, 4, 7, 8, 11}, {}},
    {"Fifths", "Simetricas", {0, 7}, {}},
    {"Octaves", "Simetricas", {0}, {}},
    // --- Del mundo
    {"Double harmonic", "Mundo", {0, 1, 4, 5, 7, 8, 11}, {}},
    {"Hungarian minor", "Mundo", {0, 2, 3, 6, 7, 8, 11}, {}},
    {"Raag Todi", "Mundo", {0, 1, 3, 6, 7, 8, 11}, {}},
    {"Prometheus", "Mundo", {0, 2, 4, 6, 9, 10}, {}},
    {"Enigmatic", "Mundo", {0, 1, 4, 6, 8, 10, 11}, {}},
    // --- Microtonales
    {"Slendro", "Microtonales", {0, 2.4f, 4.8f, 7.2f, 9.6f}, {}},
    {"Maqam Rast", "Microtonales", {0, 2, 3.5f, 5, 7, 9, 10.5f}, {}},
    {"Maqam Bayati", "Microtonales", {0, 1.5f, 3, 5, 7, 8, 10}, {}},
    {"Just major", "Microtonales", {0, 2.039f, 3.863f, 4.98f, 7.02f, 8.844f, 10.883f}, {}},
    {"Harmonic series", "Microtonales", {0, 2.039f, 3.863f, 5.513f, 7.02f, 8.405f, 9.688f, 10.883f}, {}},
  };
  return list;
}

inline int count() { return static_cast<int>(all().size()); }
inline int midiIndex() { return count(); }  // última opción: escala aprendida por MIDI

inline int autoWeight(float semis) {
  const int i = static_cast<int>(std::lround(semis)) % 12;
  switch (i) {
    case 0: return 255;
    case 7: return 200;
    case 3: case 4: return 160;
    case 5: case 9: return 128;
    case 2: case 10: case 11: return 112;
    default: return 96;
  }
}

// Máscara de 12 notas (relativa a la tónica) para el teclado del panel.
inline int mask(int index) {
  if (index < 0 || index >= count()) return 0;
  const auto& d = all()[static_cast<size_t>(index)];
  int m = 0;
  for (size_t k = 0; k < d.semis.size(); ++k) {
    const int w = d.weights.empty() ? 255 : d.weights[k];
    if (w >= 64) m |= 1 << (static_cast<int>(std::lround(d.semis[k])) % 12);
  }
  return m;
}

}  // namespace scales
