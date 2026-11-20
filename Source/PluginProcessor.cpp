#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"
#include "Presets.h"
#include "Scales.h"

using APF = juce::AudioParameterFloat;
using APC = juce::AudioParameterChoice;
using APB = juce::AudioParameterBool;
using NR = juce::NormalisableRange<float>;

juce::StringArray ResonatorIntoTextureProcessor::plaitsEngineNames() {
  return {"VA-VCF", "Phase dist.", "6-op FM I", "6-op FM II", "6-op FM III",
          "Wave terrain", "String machine", "Chiptune",
          "Virtual analog", "Waveshaping", "2-op FM", "Formant grain",
          "Additive", "Wavetable", "Chords", "Speech",
          "Swarm", "Filtered noise", "Particles", "Inharm. string",
          "Modal", "Bass drum", "Snare drum", "Hi-hat"};
}

juce::StringArray ResonatorIntoTextureProcessor::texModeNames() {
  return {"Granular", "Stretch", "Looping delay", "Spectral", "Oliverb", "Resonestor",
          "Trig drums", "Data bender"};
}

juce::StringArray ResonatorIntoTextureProcessor::scaleNames() {
  juce::StringArray a;
  for (const auto& d : scales::all()) a.add(d.name);
  a.add("MIDI");
  return a;
}

juce::AudioProcessorValueTreeState::ParameterLayout ResonatorIntoTextureProcessor::createLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> v;
  auto F = [&](int i, const char* name, float lo, float hi, float def, const char* unit = "") {
    v.push_back(std::make_unique<APF>(juce::ParameterID{pid::id(i), 1}, name, NR(lo, hi), def,
                                      juce::AudioParameterFloatAttributes().withLabel(unit)));
  };
  auto C = [&](int i, const char* name, juce::StringArray choices, int def) {
    v.push_back(std::make_unique<APC>(juce::ParameterID{pid::id(i), 1}, name, choices, def));
  };
  auto B = [&](int i, const char* name, bool def) {
    v.push_back(std::make_unique<APB>(juce::ParameterID{pid::id(i), 1}, name, def));
  };

  F(pid::IN_GAIN, "Input gain", -24.0f, 24.0f, 0.0f, "dB");
  F(pid::OUT_GAIN, "Output gain", -24.0f, 12.0f, 0.0f, "dB");
  F(pid::MIX, "Mix", 0.0f, 1.0f, 1.0f);

  B(pid::R_ON, "Resonator on", true);
  F(pid::R_FREQ, "Res. frequency", 0.0f, 60.0f, 30.0f, "st");
  F(pid::R_STRUCT, "Res. structure", 0.0f, 1.0f, 0.4f);
  F(pid::R_BRIGHT, "Res. brightness", 0.0f, 1.0f, 0.6f);
  F(pid::R_DAMP, "Res. damping", 0.0f, 1.0f, 0.65f);
  F(pid::R_POS, "Res. position", 0.0f, 1.0f, 0.35f);
  C(pid::R_MODEL, "Res. model", {"Modal", "Sympathetic strings", "Inharmonic string",
                                 "FM voice", "Sympathetic quantized", "String + reverb"}, 0);
  C(pid::R_POLY, "Res. polyphony", {"1", "2", "4"}, 1);

  B(pid::C_ON, "Texture on", true);
  F(pid::C_POS, "Tex. position", 0.0f, 1.0f, 0.3f);
  F(pid::C_SIZE, "Tex. size", 0.0f, 1.0f, 0.6f);
  F(pid::C_PITCH, "Tex. pitch", -24.0f, 24.0f, 0.0f, "st");
  F(pid::C_DENS, "Tex. density", 0.0f, 1.0f, 0.65f);
  F(pid::C_TEX, "Tex. texture", 0.0f, 1.0f, 0.6f);
  F(pid::C_DRYWET, "Tex. dry/wet", 0.0f, 1.0f, 0.6f);
  F(pid::C_SPREAD, "Tex. stereo spread", 0.0f, 1.0f, 0.6f);
  F(pid::C_FB, "Tex. feedback", 0.0f, 1.0f, 0.3f);
  F(pid::C_REV, "Tex. reverb", 0.0f, 1.0f, 0.5f);
  B(pid::C_FREEZE, "Tex. freeze", false);
  C(pid::C_MODE, "Tex. mode", texModeNames(), 0);
  C(pid::C_QUALITY, "Tex. quality", {"16-bit stereo", "16-bit mono", "8-bit stereo", "8-bit mono"}, 0);
  C(pid::C_ALT1, "Tex. option A", {"1", "2", "3", "4", "5", "6", "7", "8"}, 0);
  C(pid::C_ALT2, "Tex. option B", {"1", "2", "3", "4", "5", "6"}, 0);
  C(pid::C_FX, "Tex. perf FX", {"Off", "Stutter", "Tape stop", "Filter sweep", "Bitcrush", "Wash",
                                "Reverse", "Gate", "Echo throw / Oct down"}, 0);

  B(pid::GEN_ON, "Generator", false);
  C(pid::R_EXCITER, "Res. exciter", {"Track + cables", "Cables only", "Internal + cables"}, 0);
  C(pid::R_QUANT, "Res. note quantize", {"Free", "Semitones", "Scale"}, 0);
  C(pid::P_QUANT, "Osc note quantize", {"Free", "Semitones", "Scale"}, 0);

  F(pid::M_RATE, "T rate", 0.0f, 1.0f, 0.42f);
  F(pid::M_BIAS, "T bias", 0.0f, 1.0f, 0.5f);
  F(pid::M_JITTER, "T jitter", 0.0f, 1.0f, 0.2f);
  C(pid::M_TMODEL, "T model", {"Coin toss", "Ratios", "Drums", "Independent",
                               "Divider", "Three states", "Markov"}, 0);
  C(pid::M_TRANGE, "T range", {"x0.25", "x1", "x4"}, 1);
  F(pid::M_DV, "Deja vu", 0.0f, 1.0f, 0.3f);
  F(pid::M_LEN, "Loop length", 0.0f, 1.0f, 0.5f);
  B(pid::M_DVT, "Deja vu T", true);
  B(pid::M_DVX, "Deja vu X", true);
  F(pid::M_SPREAD, "X spread", 0.0f, 1.0f, 0.5f);
  F(pid::M_XBIAS, "X bias", 0.0f, 1.0f, 0.5f);
  F(pid::M_STEPS, "X steps", 0.0f, 1.0f, 0.72f);
  C(pid::M_XRANGE, "X range", {"+2V", "+5V", "+/-5V"}, 0);
  C(pid::M_SCALE, "Scale", scaleNames(), 0);
  C(pid::M_ROOT, "Root", {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 0);
  B(pid::M_SYNC, "Host sync", false);
  C(pid::M_SYNC_DIV, "Sync division", {"1/1", "1/2", "1/4", "1/8", "1/16", "1/4T", "1/8T", "1/16T"}, 3);

  C(pid::P_ENGINE, "Osc model", plaitsEngineNames(), 8);
  F(pid::P_PITCH, "Osc pitch", -24.0f, 24.0f, 0.0f, "st");
  F(pid::P_HARM, "Osc harmonics", 0.0f, 1.0f, 0.5f);
  F(pid::P_TIMBRE, "Osc timbre", 0.0f, 1.0f, 0.5f);
  F(pid::P_MORPH, "Osc morph", 0.0f, 1.0f, 0.5f);
  F(pid::P_DECAY, "Osc decay", 0.0f, 1.0f, 0.55f);
  F(pid::P_LPG, "Osc LPG colour", 0.0f, 1.0f, 0.5f);
  F(pid::P_LEVEL, "Osc level", 0.0f, 1.0f, 0.7f);

  // Atenuversores de cada entrada de CV
  static const char* modName[] = {"I/O", "Rnd", "Osc", "Res", "Tex"};
  for (int k = 0; k < pid::NUM_ATT; ++k) {
    const auto& info = port::inInfo(pid::attPort(k));
    const int in = pid::attPort(k);
    if (pid::isAudioInput(in)) {
      const juce::String name = juce::String(modName[info.module]) + " " +
                                (info.module == port::MOD_IO ? juce::String("to DAW ") : juce::String()) +
                                info.label + " level";
      v.push_back(std::make_unique<APF>(juce::ParameterID{pid::id(pid::ATT_BASE + k), 1}, name,
                                        NR(0.0f, 1.0f), 1.0f));
    } else {
      const juce::String name = juce::String(modName[info.module]) + " " + info.label + " CV amt";
      v.push_back(std::make_unique<APF>(juce::ParameterID{pid::id(pid::ATT_BASE + k), 1}, name,
                                        NR(-1.0f, 1.0f), 0.0f));
    }
  }

  return {v.begin(), v.end()};
}

ResonatorIntoTextureProcessor::ResonatorIntoTextureProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout()) {
  raw_.resize(pid::COUNT);
  for (int i = 0; i < pid::COUNT; ++i) {
    raw_[static_cast<size_t>(i)] = apvts.getRawParameterValue(pid::id(i));
    jassert(raw_[static_cast<size_t>(i)] != nullptr);
  }
}

bool ResonatorIntoTextureProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  const auto out = layouts.getMainOutputChannelSet();
  if (out != juce::AudioChannelSet::stereo()) return false;
  const auto in = layouts.getMainInputChannelSet();
  return in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::mono() ||
         in.isDisabled();
}

void ResonatorIntoTextureProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  const int mask = engine_.learnedMask();
  const int root = engine_.learnedRoot();
  engine_.prepare(sampleRate, samplesPerBlock);
  engine_.setLearned(mask, root);
  setLatencySamples(engine_.latencySamples());
  scratch_.setSize(2, samplesPerBlock);
}

void ResonatorIntoTextureProcessor::readParams(ric::EngineParams& p) {
  auto i = [&](int idx) { return static_cast<int>(get(idx) + 0.5f); };
  p.inputGain = juce::Decibels::decibelsToGain(get(pid::IN_GAIN));
  p.outputGain = juce::Decibels::decibelsToGain(get(pid::OUT_GAIN));
  p.mix = get(pid::MIX);

  p.ringsOn = get(pid::R_ON) > 0.5f;
  p.ringsFreq = get(pid::R_FREQ);
  p.structure = get(pid::R_STRUCT);
  p.brightness = get(pid::R_BRIGHT);
  p.damping = get(pid::R_DAMP);
  p.position = get(pid::R_POS);
  p.ringsModel = i(pid::R_MODEL);
  static const int polys[3] = {1, 2, 4};
  p.ringsPoly = polys[juce::jlimit(0, 2, i(pid::R_POLY))];

  p.cloudsOn = get(pid::C_ON) > 0.5f;
  p.cPosition = get(pid::C_POS);
  p.cSize = get(pid::C_SIZE);
  p.cPitch = get(pid::C_PITCH);
  p.cDensity = get(pid::C_DENS);
  p.cTexture = get(pid::C_TEX);
  p.cDryWet = get(pid::C_DRYWET);
  p.cSpread = get(pid::C_SPREAD);
  p.cFeedback = get(pid::C_FB);
  p.cReverb = get(pid::C_REV);
  p.cFreeze = get(pid::C_FREEZE) > 0.5f;
  p.cMode = i(pid::C_MODE);
  p.cQuality = i(pid::C_QUALITY);

  p.genOn = get(pid::GEN_ON) > 0.5f;
  p.exciter = i(pid::R_EXCITER);
  p.rQuant = i(pid::R_QUANT);
  p.pQuant = i(pid::P_QUANT);
  p.cAlt1 = i(pid::C_ALT1);
  p.cAlt2 = i(pid::C_ALT2);
  p.cFx = i(pid::C_FX);

  p.tRate = get(pid::M_RATE);
  p.tBias = get(pid::M_BIAS);
  p.tJitter = get(pid::M_JITTER);
  p.tModel = i(pid::M_TMODEL);
  p.tRange = i(pid::M_TRANGE);
  p.dejaVu = get(pid::M_DV);
  p.dvLength = get(pid::M_LEN);
  p.dvT = get(pid::M_DVT) > 0.5f;
  p.dvX = get(pid::M_DVX) > 0.5f;
  p.xSpread = get(pid::M_SPREAD);
  p.xBias = get(pid::M_XBIAS);
  p.xSteps = get(pid::M_STEPS);
  p.xRange = i(pid::M_XRANGE);
  p.scale = i(pid::M_SCALE);
  p.root = i(pid::M_ROOT);
  p.sync = get(pid::M_SYNC) > 0.5f;
  p.syncDiv = i(pid::M_SYNC_DIV);

  p.engine = i(pid::P_ENGINE);
  p.pPitch = get(pid::P_PITCH);
  p.harmonics = get(pid::P_HARM);
  p.timbre = get(pid::P_TIMBRE);
  p.morph = get(pid::P_MORPH);
  p.decay = get(pid::P_DECAY);
  p.lpgColour = get(pid::P_LPG);
  p.pLevel = get(pid::P_LEVEL);
  for (int k = 0; k < pid::NUM_ATT; ++k) p.att[pid::attPort(k)] = get(pid::ATT_BASE + k);
}

void ResonatorIntoTextureProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;

  const int pm = pendingMask_.exchange(-1);
  const int pr = pendingRoot_.exchange(-1);
  if (pm >= 0) engine_.setLearned(pm, pr >= 12 ? -1 : pr);

  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    if (msg.isNoteOn()) engine_.noteOn(msg.getNoteNumber());
    else if (msg.isNoteOff()) engine_.noteOff(msg.getNoteNumber());
    else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine_.allNotesOff();
  }

  readParams(params_);

  bool valid = false, playing = false;
  double ppq = 0.0, bpm = 120.0;
  if (auto* ph = getPlayHead()) {
    if (auto pos = ph->getPosition()) {
      playing = pos->getIsPlaying();
      if (auto b = pos->getBpm()) bpm = *b;
      if (auto q = pos->getPpqPosition()) { ppq = *q; valid = true; }
    }
  }
  engine_.setHostTime(valid, playing, ppq, bpm);

  const int n = buffer.getNumSamples();
  const int numIn = getTotalNumInputChannels();
  if (scratch_.getNumSamples() < n) scratch_.setSize(2, n, false, false, true);

  const float* inL = numIn > 0 ? buffer.getReadPointer(0) : nullptr;
  const float* inR = numIn > 1 ? buffer.getReadPointer(1) : inL;
  if (inL == nullptr) {
    scratch_.clear();
    inL = scratch_.getReadPointer(0);
    inR = inL;
  }

  // El motor admite que salida y entrada compartan memoria.
  engine_.process(inL, inR, buffer.getWritePointer(0), buffer.getWritePointer(1), n, params_);
  midi.clear();
}

juce::AudioProcessorEditor* ResonatorIntoTextureProcessor::createEditor() {
  return new ResonatorIntoTextureEditor(*this);
}

void ResonatorIntoTextureProcessor::getStateInformation(juce::MemoryBlock& destData) {
  auto state = apvts.copyState();
  state.setProperty("midiMask", engine_.learnedMask(), nullptr);
  state.setProperty("midiRoot", engine_.learnedRoot(), nullptr);
  state.setProperty("program", currentProgram_, nullptr);
  state.setProperty("presetName", currentPresetName_, nullptr);
  {
    float dp[72];
    engine_.getDrumParams(dp);
    juce::StringArray a;
    for (float v : dp) a.add(juce::String(v, 4));
    state.setProperty("drumPrm", a.joinIntoString(","), nullptr);
  }
  state.removeChild(state.getChildWithName("CABLES"), nullptr);
  juce::ValueTree cablesTree("CABLES");
  for (const auto& c : getCables()) {
    juce::ValueTree t("C");
    t.setProperty("out", port::outInfo(c.out).key, nullptr);
    t.setProperty("in", port::inInfo(c.in).key, nullptr);
    t.setProperty("col", c.colour, nullptr);
    cablesTree.appendChild(t, nullptr);
  }
  state.appendChild(cablesTree, nullptr);
  if (auto xml = state.createXml()) copyXmlToBinary(*xml, destData);
}

void ResonatorIntoTextureProcessor::setStateInformation(const void* data, int sizeInBytes) {
  if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
    if (xml->hasTagName(apvts.state.getType())) {
      auto tree = juce::ValueTree::fromXml(*xml);
      const int mask = tree.getProperty("midiMask", 0);
      const int root = tree.getProperty("midiRoot", -1);
      currentProgram_ = tree.getProperty("program", 0);
      currentPresetName_ = tree.getProperty("presetName", "").toString();
      {
        juce::StringArray a;
        a.addTokens(tree.getProperty("drumPrm", "").toString(), ",", "");
        if (a.size() == 72) {
          float dp[72];
          for (int k = 0; k < 72; ++k) dp[k] = a[k].getFloatValue();
          engine_.setDrumParams(dp);
        }
      }
      std::vector<port::Cable> cables;
      const auto cablesTree = tree.getChildWithName("CABLES");
      for (int i = 0; i < cablesTree.getNumChildren(); ++i) {
        const auto t = cablesTree.getChild(i);
        port::Cable c;
        c.out = port::findOut(t.getProperty("out").toString().toRawUTF8());
        c.in = port::findIn(t.getProperty("in").toString().toRawUTF8());
        c.colour = t.getProperty("col", 0);
        if (c.out >= 0 && c.in >= 0) cables.push_back(c);
      }
      tree.removeChild(cablesTree, nullptr);
      apvts.replaceState(tree);
      // replaceState() no reescribe un parámetro si su valor desnormalizado
      // coincide (p. ej. un bool con valor interno 0.26): se fuerza aquí.
      for (int i = 0; i < pid::COUNT; ++i) {
        auto* prm = apvts.getParameter(pid::id(i));
        const auto child = tree.getChildWithProperty("id", pid::id(i));
        if (prm == nullptr || !child.isValid()) continue;
        const float v = prm->convertTo0to1(static_cast<float>(child.getProperty("value")));
        if (std::abs(prm->getValue() - v) > 1.0e-6f) prm->setValueNotifyingHost(v);
      }
      setCablesInternal(cables);
      pendingRoot_.store(root < 0 ? 12 : root);
      pendingMask_.store(mask);
      engine_.status().midiMask.store(mask);
      engine_.status().midiRoot.store(root);
    }
  }
}

// ----------------------------------------------------------------- cables

std::vector<port::Cable> ResonatorIntoTextureProcessor::getCables() const {
  std::lock_guard<std::mutex> lock(cablesMutex_);
  return cables_;
}

void ResonatorIntoTextureProcessor::setCablesInternal(std::vector<port::Cable> cables) {
  {
    std::lock_guard<std::mutex> lock(cablesMutex_);
    cables_ = std::move(cables);
    for (const auto& c : cables_) nextColour_ = std::max(nextColour_, c.colour + 1);
    engine_.setCables(cables_);
  }
  cablesVersion_.fetch_add(1);
}

void ResonatorIntoTextureProcessor::addCable(int out, int in) {
  auto cables = getCables();
  for (const auto& c : cables)
    if (c.out == out && c.in == in) return;
  port::Cable c;
  c.out = out;
  c.in = in;
  c.colour = nextColour_++;
  cables.push_back(c);
  // Si el atenuversor está a cero, se abre para que el cable se note.
  const int ap = pid::attParamForPort(in);
  if (ap >= 0 && !pid::isAudioInput(in) && std::abs(get(ap)) < 0.001f) {
    const bool pitch = in == port::IN_P_FM || in == port::IN_R_FM || in == port::IN_C_PITCH;
    setParamReal(ap, pitch ? 1.0f : 0.5f);
  }
  setCablesInternal(cables);
}

void ResonatorIntoTextureProcessor::removeCable(int out, int in) {
  auto cables = getCables();
  cables.erase(std::remove_if(cables.begin(), cables.end(),
                              [&](const port::Cable& c) { return c.out == out && c.in == in; }),
               cables.end());
  setCablesInternal(cables);
}

void ResonatorIntoTextureProcessor::removeCablesAtOutput(int out) {
  auto cables = getCables();
  cables.erase(std::remove_if(cables.begin(), cables.end(),
                              [&](const port::Cable& c) { return c.out == out; }),
               cables.end());
  setCablesInternal(cables);
}

void ResonatorIntoTextureProcessor::removeCablesAtInput(int in) {
  auto cables = getCables();
  cables.erase(std::remove_if(cables.begin(), cables.end(),
                              [&](const port::Cable& c) { return c.in == in; }),
               cables.end());
  setCablesInternal(cables);
}

void ResonatorIntoTextureProcessor::clearCables() { setCablesInternal({}); }

void ResonatorIntoTextureProcessor::ensureGenCables() {
  auto cables = getCables();
  for (const auto& c : cables) {
    const auto mo = port::outInfo(c.out).module;
    const auto mi = port::inInfo(c.in).module;
    if (mo == port::MOD_MARBLES || mo == port::MOD_PLAITS || mi == port::MOD_MARBLES ||
        mi == port::MOD_PLAITS)
      return;
  }
  const std::pair<int, int> std_[] = {
      {port::OUT_P_OUT, port::IN_R_IN},
      {port::OUT_M_T1, port::IN_P_TRIG}, {port::OUT_M_X1, port::IN_P_VOCT},
      {port::OUT_M_T3, port::IN_R_STRUM}, {port::OUT_M_X2, port::IN_R_VOCT},
      {port::OUT_M_Y, port::IN_P_TIMBRE}, {port::OUT_M_Y, port::IN_C_POS}};
  for (auto [o, i] : std_) {
    port::Cable c;
    c.out = o;
    c.in = i;
    c.colour = nextColour_++;
    cables.push_back(c);
  }
  setParamReal(pid::attParamForPort(port::IN_P_TIMBRE), 0.35f);
  setParamReal(pid::attParamForPort(port::IN_C_POS), 0.2f);
  setCablesInternal(cables);
}

void ResonatorIntoTextureProcessor::setParamReal(int index, float value) {
  if (auto* prm = apvts.getParameter(pid::id(index))) {
    prm->beginChangeGesture();
    prm->setValueNotifyingHost(prm->convertTo0to1(value));
    prm->endChangeGesture();
  }
}

// --------------------------------------------------------------- programas

int ResonatorIntoTextureProcessor::getNumPrograms() {
  return static_cast<int>(presets::all().size()) + 1;
}

const juce::String ResonatorIntoTextureProcessor::getProgramName(int index) {
  if (index <= 0) return "Init";
  const auto& list = presets::all();
  if (index > static_cast<int>(list.size())) return {};
  return juce::String(index).paddedLeft('0', 2) + "  " +
         juce::String::fromUTF8(list[static_cast<size_t>(index - 1)].name);
}

void ResonatorIntoTextureProcessor::setCurrentProgram(int index) {
  const auto& list = presets::all();
  index = juce::jlimit(0, static_cast<int>(list.size()), index);
  currentProgram_ = index;
  currentPresetName_ = {};
  // Todo a valores por defecto
  for (int i = 0; i < pid::COUNT; ++i) {
    if (auto* prm = apvts.getParameter(pid::id(i))) {
      prm->beginChangeGesture();
      prm->setValueNotifyingHost(prm->getDefaultValue());
      prm->endChangeGesture();
    }
  }
  std::vector<port::Cable> cables;
  if (index > 0) {
    const auto& pr = list[static_cast<size_t>(index - 1)];
    for (const auto& [idx, value] : pr.values) setParamReal(idx, value);
    int colour = 0;
    for (const auto& pc : pr.cables) {
      port::Cable c;
      c.out = port::findOut(pc.out);
      c.in = port::findIn(pc.in);
      c.colour = colour++;
      if (c.out < 0 || c.in < 0) { jassertfalse; continue; }
      cables.push_back(c);
      const int ap = pid::attParamForPort(c.in);
      if (ap >= 0 && pc.att != 0.0f) setParamReal(ap, pc.att);
    }
  }
  nextColour_ = static_cast<int>(cables.size());
  setCablesInternal(cables);
}

// --------------------------------------------------------- presets usuario

juce::File ResonatorIntoTextureProcessor::userPresetDir() {
  auto d = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("ResonatorIntoTexture")
               .getChildFile("Presets");
  d.createDirectory();
  return d;
}

juce::Array<juce::File> ResonatorIntoTextureProcessor::userPresets() {
  auto files = userPresetDir().findChildFiles(juce::File::findFiles, false, "*.ritpreset");
  files.sort();
  return files;
}

bool ResonatorIntoTextureProcessor::saveUserPreset(const juce::File& file, const juce::String& name) {
  currentPresetName_ = name;
  juce::MemoryBlock mb;
  getStateInformation(mb);
  auto xml = getXmlFromBinary(mb.getData(), static_cast<int>(mb.getSize()));
  if (!xml) return false;
  xml->removeAttribute("zoom");
  return xml->writeTo(file);
}

bool ResonatorIntoTextureProcessor::loadUserPreset(const juce::File& file) {
  auto xml = juce::XmlDocument::parse(file);
  if (!xml || !xml->hasTagName(apvts.state.getType())) return false;
  const auto zoom = apvts.state.getProperty("zoom", 1.0);
  xml->setAttribute("zoom", static_cast<double>(zoom));
  xml->setAttribute("presetName", file.getFileNameWithoutExtension());
  juce::MemoryBlock mb;
  copyXmlToBinary(*xml, mb);
  setStateInformation(mb.getData(), static_cast<int>(mb.getSize()));
  currentPresetName_ = file.getFileNameWithoutExtension();
  return true;
}

juce::String ResonatorIntoTextureProcessor::currentPatchName() {
  if (currentPresetName_.isNotEmpty()) return currentPresetName_;
  return getProgramName(currentProgram_);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new ResonatorIntoTextureProcessor();
}
