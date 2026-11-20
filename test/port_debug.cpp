#include "../Source/PluginProcessor.h"
#include "../Source/Params.h"
int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI init;
  auto proc = std::make_unique<ResonatorIntoTextureProcessor>();
  proc->setCurrentProgram(atoi(argv[1]));
  proc->prepareToPlay(48000, 480);
  juce::AudioBuffer<float> buf(2, 480); juce::MidiBuffer midi;
  const int outs[] = {port::OUT_M_T1, port::OUT_M_T3, port::OUT_M_X1, port::OUT_P_OUT, port::OUT_R_ODD, port::OUT_C_OUT_L};
  printf("sec   T1    T3    X1   POUT  RODD  COUT\n");
  for (int s = 0; s < 12; ++s) {
    float mx[6] = {0};
    for (int b = 0; b < 100; ++b) { buf.clear(); proc->processBlock(buf, midi);
      for (int k = 0; k < 6; ++k) mx[k] = std::max(mx[k], proc->status().outLevel[outs[k]].exchange(0.0f)); }
    printf("%3d", s); for (float v : mx) printf(" %5.3f", v); printf("\n");
  }
}
