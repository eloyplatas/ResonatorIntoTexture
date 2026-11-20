#include "../Source/PluginProcessor.h"
#include "../Source/Params.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <chrono>

struct FakeHead : juce::AudioPlayHead {
  double ppq = 0, bpm = 120; bool playing = true;
  juce::Optional<PositionInfo> getPosition() const override {
    PositionInfo p; p.setIsPlaying(playing); p.setBpm(bpm); p.setPpqPosition(ppq); return p; }
};

static void writeWav(const juce::AudioBuffer<float>& b, int sr, const juce::String& path) {
  juce::File f(path); f.deleteFile();
  juce::WavAudioFormat fmt;
  std::unique_ptr<juce::AudioFormatWriter> w(fmt.createWriterFor(new juce::FileOutputStream(f), sr, 2, 24, {}, 0));
  w->writeFromAudioSampleBuffer(b, 0, b.getNumSamples());
}

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI init;
  const int sr = 48000, block = 512; const double secs = 12.0;
  const bool writeAll = argc > 1;
  auto proc = std::make_unique<ResonatorIntoTextureProcessor>();
  FakeHead head; proc->setPlayHead(&head);
  printf("%-26s %7s %7s %7s %6s %5s %6s %6s\n", "patch", "peak", "rmsdB", "minWin", "clip%", "nan", "cpu%", "L/R dB");
  for (int prog = 0; prog < proc->getNumPrograms(); ++prog) {
    proc->setCurrentProgram(prog);
    proc->prepareToPlay(sr, block);
    const bool gen = proc->apvts.getRawParameterValue(pid::id(pid::GEN_ON))->load() > 0.5f;
    const int total = (int)(secs * sr);
    juce::AudioBuffer<float> out(2, total), buf(2, block);
    juce::MidiBuffer midi; juce::Random rnd(7);
    double ph = 0, cpu = 0; head.ppq = 0;
    for (int pos = 0; pos < total; pos += block) {
      const int n = std::min(block, total - pos);
      buf.setSize(2, n, false, false, true);
      for (int i = 0; i < n; ++i) {
        const double t = (pos + i) / (double)sr;
        float s = 0;
        if (!gen) {  // "guitarra" sintética: notas punteadas cada 0.75 s
          const double tn = fmod(t, 0.75); const int note = (int)(t / 0.75) % 4;
          const double f = 110.0 * pow(2.0, (double[]){0, 3, 7, 10}[note] / 12.0);
          ph += f / sr; s = (float)(0.5 * exp(-tn * 4.0) * (2.0 * (ph - floor(ph)) - 1.0));
          s += 0.02f * (rnd.nextFloat() - 0.5f) * (float)exp(-tn * 30.0);
        }
        buf.setSample(0, i, s); buf.setSample(1, i, s);
      }
      auto t0 = std::chrono::high_resolution_clock::now();
      proc->processBlock(buf, midi);
      cpu += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
      head.ppq += n / (double)sr * head.bpm / 60.0;
      for (int c = 0; c < 2; ++c) out.copyFrom(c, pos, buf, c, 0, n);
    }
    float peak = 0; double sum = 0; int clip = 0; bool nan = false;
    const int from = 2 * sr;
    double minWin = 1e9;  // RMS mínima en ventanas de 1 s (detecta silencios)
    for (int w = from; w + sr <= total; w += sr) {
      double ws = 0; for (int i = w; i < w + sr; ++i) ws += out.getSample(0, i) * out.getSample(0, i) + out.getSample(1, i) * out.getSample(1, i);
      minWin = std::min(minWin, sqrt(ws / (2.0 * sr)));
    }
    for (int i = from; i < total; ++i) for (int c = 0; c < 2; ++c) {
      const float v = out.getSample(c, i);
      if (!std::isfinite(v)) nan = true;
      peak = std::max(peak, std::abs(v)); sum += v * v; if (std::abs(v) > 0.99f) ++clip;
    }
    const double rms = sqrt(sum / (2.0 * (total - from)));
    double sl = 0, srr = 0; for (int i = from; i < total; ++i) { sl += out.getSample(0,i)*out.getSample(0,i); srr += out.getSample(1,i)*out.getSample(1,i); }
    const double lr = 10.0 * log10((sl + 1e-12) / (srr + 1e-12));
    printf("%-26s %7.3f %7.1f %7.1f %6.2f %5d %6.1f ", proc->getProgramName(prog).substring(0, 26).toRawUTF8(),
           peak, juce::Decibels::gainToDecibels(rms), juce::Decibels::gainToDecibels(minWin),
           100.0 * clip / (2.0 * (total - from)), (int)nan, 100.0 * cpu / secs);
    printf("%*s%6.1f\n", 0, "", lr);
    if (writeAll) writeWav(out, sr, "/home/claude/RingsIntoClouds/test/wav/p" + juce::String(prog).paddedLeft('0', 2) + ".wav");
  }
  // Presets de usuario: guardar, cambiar, cargar y comparar
  {
    proc->setCurrentProgram(5);
    proc->addCable(port::OUT_R_ODD, port::IN_C_DENS);
    proc->apvts.getParameter(pid::id(pid::C_MODE))->setValueNotifyingHost(proc->apvts.getParameter(pid::id(pid::C_MODE))->convertTo0to1(4.0f));
    auto f = ResonatorIntoTextureProcessor::userPresetDir().getChildFile("__prueba.ritpreset");
    const bool saved = proc->saveUserPreset(f, "__prueba");
    const auto cablesBefore = proc->getCables().size();
    proc->setCurrentProgram(0);
    const bool loaded = proc->loadUserPreset(f);
    const float mode = proc->apvts.getParameter(pid::id(pid::C_MODE))->convertFrom0to1(proc->apvts.getParameter(pid::id(pid::C_MODE))->getValue());
    printf("preset usuario: guardado %d cargado %d cables %zu->%zu modo %.0f nombre '%s' en %s\n", (int)saved, (int)loaded,
           cablesBefore, proc->getCables().size(), mode, proc->currentPatchName().toRawUTF8(), f.getParentDirectory().getFullPathName().toRawUTF8());
    f.deleteFile();
  }
  return 0;
}
