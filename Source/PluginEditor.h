#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "Ports.h"

class ResonatorIntoTextureEditor;

namespace ui {

enum KnobStyle { KNOB_BIG = 0, KNOB_MEDIUM, KNOB_SMALL, KNOB_DEJAVU, KNOB_MINI };

class LookAndFeel : public juce::LookAndFeel_V4 {
 public:
  LookAndFeel();
  void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos,
                        float start, float end, juce::Slider&) override;
  juce::Font getSliderPopupFont(juce::Slider&) override;
  void drawBubble(juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip,
                  const juce::Rectangle<float>& body) override;
};

void drawLed(juce::Graphics& g, juce::Point<float> c, float r, juce::Colour col, float amount);
void drawButtonCap(juce::Graphics& g, juce::Point<float> c, float r, bool down);
juce::Font labelFont(float size, bool bold = true);
juce::Colour cableColour(int index);

class Knob : public juce::Slider {
 public:
  Knob(juce::AudioProcessorValueTreeState& s, const char* id, KnobStyle style, juce::String caption);
  KnobStyle style;
  juce::String caption;
  juce::String getTextFromValue(double v) override;
 private:
  juce::AudioProcessorValueTreeState::SliderAttachment att_;
};

// Jack real: arrastrar crea / mueve cables; clic derecho los quita.
class Jack : public juce::Component, public juce::SettableTooltipClient {
 public:
  Jack(ResonatorIntoTextureEditor& ed, int index, bool isOutput);
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  juce::Point<float> centreIn(juce::Component& parent) const;
  int index() const { return index_; }
  bool isOutput() const { return isOutput_; }
  static constexpr int kW = 44, kH = 42;
  static constexpr float kJackY = 27.0f;
 private:
  ResonatorIntoTextureEditor& ed_;
  int index_;
  bool isOutput_;
};

class CycleButton : public juce::Component, public juce::SettableTooltipClient {
 public:
  using LedFn = std::function<juce::Colour(int value, int led)>;
  CycleButton(juce::RangedAudioParameter* p, juce::String caption, int numLeds, LedFn fn, bool showText);
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  int value() const;
 private:
  juce::RangedAudioParameter* param_;
  juce::String caption_;
  int numLeds_;
  LedFn ledFn_;
  bool showText_;
  bool down_ = false;
};

class StepButton : public juce::Component {
 public:
  StepButton(juce::RangedAudioParameter* p, int delta) : param_(p), delta_(delta) {}
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override { down_ = false; repaint(); }
 private:
  juce::RangedAudioParameter* param_;
  int delta_;
  bool down_ = false;
};

class GenButton : public juce::Component {
 public:
  explicit GenButton(ResonatorIntoTextureProcessor& p) : proc_(p) {}
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override { down_ = false; repaint(); }
 private:
  ResonatorIntoTextureProcessor& proc_;
  bool down_ = false;
};

// Selector genérico con menú: rótulo y opciones dependen del contexto.
class MenuBox : public juce::Component, public juce::SettableTooltipClient {
 public:
  using TextFn = std::function<juce::String()>;
  using ListFn = std::function<juce::StringArray()>;
  MenuBox(juce::RangedAudioParameter* p, TextFn caption, ListFn options, juce::Colour led);
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
 private:
  juce::RangedAudioParameter* param_;
  TextFn caption_;
  ListFn options_;
  juce::Colour led_;
};

// Selector de escala (menú agrupado).
class ScaleBox : public juce::Component, public juce::SettableTooltipClient {
 public:
  explicit ScaleBox(ResonatorIntoTextureProcessor& p);
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
 private:
  void select(int index);
  ResonatorIntoTextureProcessor& proc_;
};

// Selector de patches de fábrica.
class PatchBox : public juce::Component {
 public:
  explicit PatchBox(ResonatorIntoTextureProcessor& p) : proc_(p) {}
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
 private:
  void select(int index);
  ResonatorIntoTextureProcessor& proc_;
  std::unique_ptr<juce::FileChooser> chooser_;
};

class Panel : public juce::Component {
 public:
  Panel(ResonatorIntoTextureEditor& ed, juce::String title, juce::String subtitle, int hp);
  void paint(juce::Graphics&) override;
  void resized() override { background_ = {}; }
  virtual void tick() {}

  static constexpr int kHeight = 740;
  static constexpr float kHpPx = 21.0f;

 protected:
  virtual void paintStatic(juce::Graphics&) {}
  virtual void paintDynamic(juce::Graphics&) {}
  void caption(juce::Graphics& g, const juce::String& text, float cx, float y, float size = 10.5f);
  void groupBox(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& label = {});
  void outPlate(juce::Graphics& g, juce::Rectangle<float> r);
  static float decay(float current, float target, float k = 0.80f) { return std::max(target, current * k); }

  ResonatorIntoTextureEditor& ed_;
  ResonatorIntoTextureProcessor& proc_;
  bool dynamicCaptions_ = false;  // los rótulos de knobs se pintan en paintDynamic
  juce::OwnedArray<juce::Component> owned_;
  template <typename T>
  T* own(T* c) { owned_.add(c); addAndMakeVisible(c); return c; }
  Knob* knob(const char* id, KnobStyle st, const juce::String& cap, float cx, float cy);
  Knob* atten(int inPort, float cx, float cy);
  Jack* jackIn(int in, float cx, float cy);
  Jack* jackOut(int out, float cx, float cy);

 private:
  juce::String title_, subtitle_;
  juce::Image background_;
};

class IoPanel : public Panel {
 public:
  explicit IoPanel(ResonatorIntoTextureEditor& ed);
  void tick() override;
  void mouseDown(const juce::MouseEvent&) override;
 protected:
  void paintStatic(juce::Graphics&) override;
  void paintDynamic(juce::Graphics&) override;
 private:
  float in_ = 0, midi_ = 0;
  int lastMask_ = -2;
};

class RingsPanel : public Panel {
 public:
  explicit RingsPanel(ResonatorIntoTextureEditor& ed);
  void tick() override;
 protected:
  void paintStatic(juce::Graphics&) override;
  void paintDynamic(juce::Graphics&) override;
 private:
  float strum_ = 0;
};

class CloudsPanel : public Panel {
 public:
  explicit CloudsPanel(ResonatorIntoTextureEditor& ed);
  void tick() override;
  int mode() const;
 protected:
  void paintStatic(juce::Graphics&) override;
  void paintDynamic(juce::Graphics&) override;
 private:
  float out_ = 0;
  int shownMode_ = -1;
  juce::Component* alt1_ = nullptr;
  juce::Component* alt2_ = nullptr;
  juce::Component* fx_ = nullptr;
  juce::Array<Knob*> knobs_;
};

class MarblesPanel : public Panel {
 public:
  explicit MarblesPanel(ResonatorIntoTextureEditor& ed);
  void tick() override { repaint(); }
 protected:
  void paintStatic(juce::Graphics&) override;
  void paintDynamic(juce::Graphics&) override;
};

class PlaitsPanel : public Panel {
 public:
  explicit PlaitsPanel(ResonatorIntoTextureEditor& ed);
  void tick() override { repaint(); }
 protected:
  void paintStatic(juce::Graphics&) override;
  void paintDynamic(juce::Graphics&) override;
};

// Capa transparente que dibuja los cables por encima de los paneles.
class CableLayer : public juce::Component {
 public:
  explicit CableLayer(ResonatorIntoTextureEditor& ed) : ed_(ed) {
    setInterceptsMouseClicks(false, false);
  }
  void paint(juce::Graphics&) override;
 private:
  ResonatorIntoTextureEditor& ed_;
};

}  // namespace ui

class ResonatorIntoTextureEditor : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit ResonatorIntoTextureEditor(ResonatorIntoTextureProcessor&);
  ~ResonatorIntoTextureEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override {}

  ResonatorIntoTextureProcessor& proc() { return proc_; }
  juce::Component& content() { return content_; }

  // Registro de jacks
  void registerJack(ui::Jack* j);
  ui::Jack* jackFor(int index, bool isOutput) const {
    return isOutput ? outJacks_[index] : inJacks_[index];
  }
  bool jackVisible(const ui::Jack* j) const;

  // Arrastre de cables
  void beginDrag(ui::Jack& j, const juce::MouseEvent& e);
  void dragTo(const juce::MouseEvent& e);
  void endDrag(const juce::MouseEvent& e);
  void clearJack(ui::Jack& j);
  bool dragging() const { return dragAnchor_ != nullptr; }
  const ui::Jack* dragAnchor() const { return dragAnchor_; }
  juce::Point<float> dragPoint() const { return dragPoint_; }
  int dragColour() const { return dragColour_; }

  // Rótulo de un jack (en TRIG DRUMS los jacks de CV de texture son disparos).
  juce::String jackLabel(int index, bool isOutput) const;
  int textureMode() const;

  // Niveles de señal (0..1) leídos del motor en cada tick.
  float outLevel(int o) const { return levels_[o]; }
  float inLevel(int in) const;
  const std::vector<port::Cable>& cables() const { return cablesCache_; }

 private:
  void timerCallback() override;
  void updateLayout();

  ResonatorIntoTextureProcessor& proc_;
  ui::LookAndFeel lnf_;
  juce::TooltipWindow tooltip_{this, 700};
  juce::Component content_;
  ui::Jack* outJacks_[port::NUM_OUT] = {};
  ui::Jack* inJacks_[port::NUM_IN] = {};
  ui::MarblesPanel marbles_;
  ui::PlaitsPanel plaits_;
  ui::IoPanel io_;
  ui::RingsPanel rings_;
  ui::CloudsPanel clouds_;
  ui::CableLayer cableLayer_;

  bool genShown_ = false;
  float zoom_ = 1.0f;
  float levels_[port::NUM_OUT] = {};
  std::vector<port::Cable> cablesCache_;
  int cablesSeen_ = -1;

  ui::Jack* dragAnchor_ = nullptr;
  juce::Point<float> dragPoint_;
  int dragColour_ = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResonatorIntoTextureEditor)
};
