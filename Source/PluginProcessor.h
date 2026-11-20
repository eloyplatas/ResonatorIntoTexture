#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"

class ResonatorIntoTextureProcessor : public juce::AudioProcessor {
 public:
  ResonatorIntoTextureProcessor();
  ~ResonatorIntoTextureProcessor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "ResonatorIntoTexture"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 10.0; }

  // Programas = patches de fábrica (0 = Init).
  int getNumPrograms() override;
  int getCurrentProgram() override { return currentProgram_; }
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState apvts;
  ric::EngineStatus& status() { return engine_.status(); }

  // ---- cables (hilo de mensajes)
  std::vector<port::Cable> getCables() const;
  void addCable(int out, int in);
  void removeCable(int out, int in);
  void removeCablesAtOutput(int out);
  void removeCablesAtInput(int in);
  void clearCables();
  // Al activar GEN sin cables del generador, crea el cableado estándar.
  void ensureGenCables();
  int cablesVersion() const { return cablesVersion_.load(); }

  // ---- presets de usuario (~/.config/ResonatorIntoTexture/Presets)
  static juce::File userPresetDir();
  static juce::Array<juce::File> userPresets();
  bool saveUserPreset(const juce::File& file, const juce::String& name);
  bool loadUserPreset(const juce::File& file);
  juce::String currentPatchName();
  bool isUserPreset() const { return currentPresetName_.isNotEmpty(); }

  static juce::StringArray texModeNames();
  static juce::StringArray plaitsEngineNames();
  static juce::StringArray scaleNames();

 private:
  static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
  void readParams(ric::EngineParams& p);
  void setCablesInternal(std::vector<port::Cable> cables);
  void setParamReal(int index, float value);

  ric::Engine engine_;
  ric::EngineParams params_;
  juce::AudioBuffer<float> scratch_;
  std::atomic<int> pendingMask_{-1}, pendingRoot_{-1};
  std::vector<std::atomic<float>*> raw_;
  float get(int index) const { return raw_[static_cast<size_t>(index)]->load(); }

  mutable std::mutex cablesMutex_;
  std::vector<port::Cable> cables_;
  std::atomic<int> cablesVersion_{0};
  int nextColour_ = 0;
  int currentProgram_ = 0;
  juce::String currentPresetName_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResonatorIntoTextureProcessor)
};
