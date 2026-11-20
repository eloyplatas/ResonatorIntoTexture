#include "PluginEditor.h"
#include "Params.h"
#include "Scales.h"

using namespace port;

namespace ui {

namespace col {
const juce::Colour panelTop(0xffdedfe1);
const juce::Colour panelBottom(0xffc6c7ca);
const juce::Colour ink(0xff1c1c1e);
const juce::Colour inkSoft(0xff4a4a4e);
const juce::Colour line(0x8c2e2e31);
const juce::Colour plate(0xff2a2a2d);
const juce::Colour green(0xff3ee068);
const juce::Colour red(0xffff3b30);
const juce::Colour amber(0xffffa21f);
const juce::Colour orange(0xfff07a1a);
const juce::Colour yellow(0xffffe14d);
const juce::Colour blue(0xff49a6ff);
}  // namespace col

juce::Font labelFont(float size, bool bold) {
  return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Colour cableColour(int index) {
  static const juce::uint32 palette[] = {0xffe8423f, 0xfff5c73d, 0xff3f9be8, 0xff4cc25a,
                                         0xfff08a24, 0xffa070e0, 0xffe9e9e9, 0xff3cc8c0};
  return juce::Colour(palette[static_cast<size_t>(((index % 8) + 8) % 8)]);
}

// ------------------------------------------------------------------ dibujo

void drawLed(juce::Graphics& g, juce::Point<float> c, float r, juce::Colour colour, float amount) {
  amount = juce::jlimit(0.0f, 1.0f, amount);
  g.setColour(juce::Colour(0xff8e8f92));
  g.fillEllipse(c.x - r - 1.2f, c.y - r - 1.2f, (r + 1.2f) * 2, (r + 1.2f) * 2);
  g.setColour(colour.interpolatedWith(juce::Colour(0xff2a2a2a), 0.78f));
  g.fillEllipse(c.x - r, c.y - r, r * 2, r * 2);
  if (amount > 0.01f) {
    juce::ColourGradient halo(colour.withAlpha(0.55f * amount), c.x, c.y,
                              colour.withAlpha(0.0f), c.x + r * 3.2f, c.y, true);
    g.setGradientFill(halo);
    g.fillEllipse(c.x - r * 3.2f, c.y - r * 3.2f, r * 6.4f, r * 6.4f);
    juce::ColourGradient body(colour.brighter(0.6f).withAlpha(amount), c.x - r * 0.3f, c.y - r * 0.3f,
                              colour.withAlpha(amount), c.x + r, c.y + r, true);
    g.setGradientFill(body);
    g.fillEllipse(c.x - r, c.y - r, r * 2, r * 2);
  }
  g.setColour(juce::Colours::white.withAlpha(0.35f));
  g.fillEllipse(c.x - r * 0.55f, c.y - r * 0.65f, r * 0.6f, r * 0.45f);
}

void drawButtonCap(juce::Graphics& g, juce::Point<float> c, float r, bool down) {
  g.setColour(juce::Colours::black.withAlpha(0.3f));
  g.fillEllipse(c.x - r - 1.5f, c.y - r + (down ? 0.5f : 1.5f), (r + 1.5f) * 2, (r + 1.5f) * 2);
  g.setColour(juce::Colour(0xff8c8d90));
  g.fillEllipse(c.x - r - 1.5f, c.y - r - 1.5f, (r + 1.5f) * 2, (r + 1.5f) * 2);
  juce::ColourGradient cap(down ? juce::Colour(0xff1b1b1d) : juce::Colour(0xff4a4a4e), c.x, c.y - r,
                           juce::Colour(0xff151517), c.x, c.y + r, false);
  g.setGradientFill(cap);
  g.fillEllipse(c.x - r, c.y - r, r * 2, r * 2);
}

static void drawJackBody(juce::Graphics& g, juce::Point<float> c, juce::Colour glow, float amount) {
  juce::Path hex;
  const float R = 10.0f;
  for (int i = 0; i < 6; ++i) {
    const float a = juce::MathConstants<float>::pi / 3.0f * static_cast<float>(i) + 0.5236f;
    const juce::Point<float> p(c.x + R * std::cos(a), c.y + R * std::sin(a));
    if (i == 0) hex.startNewSubPath(p); else hex.lineTo(p);
  }
  hex.closeSubPath();
  g.setColour(juce::Colours::black.withAlpha(0.25f));
  g.fillPath(hex, juce::AffineTransform::translation(0.0f, 1.5f));
  juce::ColourGradient metal(juce::Colour(0xffeeeff1), c.x, c.y - R, juce::Colour(0xff9a9b9f), c.x, c.y + R, false);
  g.setGradientFill(metal);
  g.fillPath(hex);
  g.setColour(juce::Colour(0xff6d6e72));
  g.strokePath(hex, juce::PathStrokeType(0.7f));
  if (amount > 0.02f) {
    g.setColour(glow.withAlpha(juce::jlimit(0.0f, 1.0f, amount)));
    g.drawEllipse(c.x - 6.6f, c.y - 6.6f, 13.2f, 13.2f, 2.0f);
  }
  g.setColour(juce::Colour(0xff0d0d0e));
  g.fillEllipse(c.x - 4.8f, c.y - 4.8f, 9.6f, 9.6f);
  g.setColour(juce::Colour(0xff3a3a3d));
  g.drawEllipse(c.x - 4.8f, c.y - 4.8f, 9.6f, 9.6f, 0.8f);
}

// ------------------------------------------------------------ LookAndFeel

LookAndFeel::LookAndFeel() {
  setColour(juce::BubbleComponent::backgroundColourId, juce::Colour(0xee1c1c1e));
  setColour(juce::BubbleComponent::outlineColourId, juce::Colour(0x00000000));
  setColour(juce::TooltipWindow::textColourId, juce::Colours::white);
  setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xee1c1c1e));
}

juce::Font LookAndFeel::getSliderPopupFont(juce::Slider&) { return labelFont(12.0f); }

void LookAndFeel::drawBubble(juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&,
                             const juce::Rectangle<float>& body) {
  g.setColour(juce::Colour(0xee1c1c1e));
  g.fillRoundedRectangle(body, 4.0f);
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                                   float start, float end, juce::Slider& s) {
  auto* knob = dynamic_cast<Knob*>(&s);
  const KnobStyle style = knob ? knob->style : KNOB_MEDIUM;
  const juce::Rectangle<float> area(static_cast<float>(x), static_cast<float>(y),
                                    static_cast<float>(w), static_cast<float>(h));
  const auto c = area.getCentre();
  const float r = std::min(area.getWidth(), area.getHeight()) * 0.5f - (style == KNOB_MINI ? 2.5f : 4.0f);
  const float angle = start + pos * (end - start);
  auto pointer = [&](float r0, float r1, float thickness, juce::Colour colour) {
    const juce::Point<float> a(c.x + r0 * std::sin(angle), c.y - r0 * std::cos(angle));
    const juce::Point<float> b(c.x + r1 * std::sin(angle), c.y - r1 * std::cos(angle));
    g.setColour(colour);
    g.drawLine({a, b}, thickness);
  };

  if (style == KNOB_SMALL || style == KNOB_MINI) {
    if (style == KNOB_MINI) {
      // Marca de centro (atenuversor bipolar)
      g.setColour(col::ink.withAlpha(0.7f));
      g.drawLine(c.x, c.y - r - 4.0f, c.x, c.y - r - 1.0f, 1.2f);
    }
    g.setColour(juce::Colours::black.withAlpha(0.32f));
    g.fillEllipse(c.x - r, c.y - r + 1.8f, r * 2, r * 2);
    juce::ColourGradient body(juce::Colour(0xff4b4b50), c.x, c.y - r, juce::Colour(0xff151517), c.x, c.y + r, false);
    g.setGradientFill(body);
    g.fillEllipse(c.x - r, c.y - r, r * 2, r * 2);
    g.setColour(juce::Colour(0xff0a0a0b));
    g.drawEllipse(c.x - r, c.y - r, r * 2, r * 2, 1.0f);
    g.setColour(juce::Colour(0xff2b2b2e));
    g.fillEllipse(c.x - r * 0.62f, c.y - r * 0.62f, r * 1.24f, r * 1.24f);
    pointer(r * 0.15f, r * 0.9f, style == KNOB_MINI ? 1.7f : 2.0f, juce::Colour(0xfff2f2f2));
    return;
  }

  g.setColour(juce::Colours::black.withAlpha(0.22f));
  g.fillEllipse(c.x - r - 1.5f, c.y - r + 3.0f, (r + 1.5f) * 2, (r + 1.5f) * 2);
  g.setColour(juce::Colours::black.withAlpha(0.12f));
  g.fillEllipse(c.x - r - 3.0f, c.y - r + 5.0f, (r + 3.0f) * 2, (r + 3.0f) * 2);
  juce::ColourGradient skirt(juce::Colour(0xfffdfcf9), c.x, c.y - r, juce::Colour(0xffc9c6bf), c.x, c.y + r, false);
  g.setGradientFill(skirt);
  g.fillEllipse(c.x - r, c.y - r, r * 2, r * 2);
  g.setColour(juce::Colour(0xff9d9a94));
  g.drawEllipse(c.x - r, c.y - r, r * 2, r * 2, 0.9f);
  const float rc = r * 0.80f;
  juce::ColourGradient cap(juce::Colour(0xffffffff), c.x - rc * 0.4f, c.y - rc,
                           juce::Colour(0xffe0ddd6), c.x + rc * 0.4f, c.y + rc, false);
  g.setGradientFill(cap);
  g.fillEllipse(c.x - rc, c.y - rc, rc * 2, rc * 2);
  g.setColour(juce::Colour(0x30000000));
  g.drawEllipse(c.x - rc, c.y - rc, rc * 2, rc * 2, 0.7f);

  if (style == KNOB_DEJAVU) {
    g.setColour(col::orange.withAlpha(0.85f));
    g.drawEllipse(c.x - r - 3.5f, c.y - r - 3.5f, (r + 3.5f) * 2, (r + 3.5f) * 2, 1.6f);
    pointer(r * 0.12f, r * 0.94f, 3.2f, col::orange.darker(0.15f));
  } else {
    pointer(r * 0.12f, r * 0.94f, style == KNOB_BIG ? 3.0f : 2.6f, juce::Colour(0xff151515));
  }
}

// ------------------------------------------------------------------- Knob

Knob::Knob(juce::AudioProcessorValueTreeState& s, const char* id, KnobStyle st, juce::String cap)
    : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox),
      style(st), caption(std::move(cap)), att_(s, id, *this) {
  setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
  setPopupDisplayEnabled(true, true, nullptr, 900);
  setMouseDragSensitivity(style == KNOB_SMALL || style == KNOB_MINI ? 180 : 260);
  if (auto* p = s.getParameter(id)) {
    setDoubleClickReturnValue(true, p->convertFrom0to1(p->getDefaultValue()));
    setTooltip(p->getName(64));
  }
}

juce::String Knob::getTextFromValue(double v) {
  auto t = juce::Slider::getTextFromValue(v);
  if (style == KNOB_MINI)
    return (caption == "LEVEL" ? "Nivel " : "CV ") + juce::String(juce::roundToInt(v * 100.0)) + " %";
  return caption.isNotEmpty() ? caption + "  " + t : t;
}

// ------------------------------------------------------------------- Jack

Jack::Jack(ResonatorIntoTextureEditor& ed, int index, bool isOutput)
    : ed_(ed), index_(index), isOutput_(isOutput) {
  setSize(kW, kH);
  setTooltip(isOutput ? juce::String("Salida ") + outInfo(index).label +
                            "  -  arrastra hasta una entrada; clic derecho: quitar cables"
                      : juce::String("Entrada ") + inInfo(index).label +
                            "  -  arrastra desde una salida; clic derecho: quitar cables");
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

juce::Point<float> Jack::centreIn(juce::Component& parent) const {
  return parent.getLocalPoint(this, juce::Point<float>(kW * 0.5f, kJackY));
}

void Jack::paint(juce::Graphics& g) {
  const juce::String label = ed_.jackLabel(index_, isOutput_);
  g.setColour(isOutput_ ? juce::Colours::white.withAlpha(0.92f) : col::ink);
  g.setFont(labelFont(8.5f));
  g.drawText(label, 0, 2, kW, 10, juce::Justification::centred);
  float level = isOutput_ ? ed_.outLevel(index_) : ed_.inLevel(index_);
  const auto glow = isOutput_ ? col::green : col::amber;
  drawJackBody(g, {kW * 0.5f, kJackY}, glow, std::min(1.0f, level * 2.5f));
}

void Jack::mouseDown(const juce::MouseEvent& e) {
  if (e.mods.isPopupMenu()) {
    ed_.clearJack(*this);
    return;
  }
  ed_.beginDrag(*this, e);
}
void Jack::mouseDrag(const juce::MouseEvent& e) { ed_.dragTo(e); }
void Jack::mouseUp(const juce::MouseEvent& e) { ed_.endDrag(e); }

// ------------------------------------------------------------- CycleButton

CycleButton::CycleButton(juce::RangedAudioParameter* p, juce::String caption, int numLeds, LedFn fn,
                         bool showText)
    : param_(p), caption_(std::move(caption)), numLeds_(numLeds), ledFn_(std::move(fn)),
      showText_(showText) {
  setTooltip(p->getName(64) + "  (clic: siguiente, clic derecho: anterior)");
}

int CycleButton::value() const {
  const int steps = std::max(2, param_->getNumSteps());
  return juce::roundToInt(param_->getValue() * static_cast<float>(steps - 1));
}

void CycleButton::paint(juce::Graphics& g) {
  g.setColour(col::ink);
  g.setFont(labelFont(10.0f));
  g.drawText(caption_, 0, 0, getWidth(), 12, juce::Justification::left);
  drawButtonCap(g, {10.0f, 23.0f}, 7.5f, down_);
  const int v = value();
  for (int i = 0; i < numLeds_; ++i) {
    const auto colour = ledFn_ ? ledFn_(v, i) : juce::Colours::transparentBlack;
    drawLed(g, {29.0f + static_cast<float>(i) * 13.0f, 23.0f}, 3.6f,
            colour.isTransparent() ? col::green : colour, colour.isTransparent() ? 0.0f : 1.0f);
  }
  if (showText_) {
    g.setColour(col::inkSoft);
    g.setFont(labelFont(9.5f, false));
    g.drawText(param_->getCurrentValueAsText(), 0, 33, getWidth(), 12, juce::Justification::left);
  }
}

void CycleButton::mouseDown(const juce::MouseEvent& e) {
  down_ = true;
  const int steps = std::max(2, param_->getNumSteps());
  int v = value() + (e.mods.isPopupMenu() ? -1 : 1);
  if (v < 0) v = steps - 1;
  if (v >= steps) v = 0;
  param_->beginChangeGesture();
  param_->setValueNotifyingHost(static_cast<float>(v) / static_cast<float>(steps - 1));
  param_->endChangeGesture();
  repaint();
}

void CycleButton::mouseUp(const juce::MouseEvent&) {
  down_ = false;
  repaint();
}

// -------------------------------------------------------------- StepButton

void StepButton::paint(juce::Graphics& g) {
  drawButtonCap(g, getLocalBounds().toFloat().getCentre(), 8.0f, down_);
}

void StepButton::mouseDown(const juce::MouseEvent&) {
  down_ = true;
  const int steps = param_->getNumSteps();
  int v = juce::roundToInt(param_->getValue() * static_cast<float>(steps - 1)) + delta_;
  v = (v + steps) % steps;
  param_->beginChangeGesture();
  param_->setValueNotifyingHost(static_cast<float>(v) / static_cast<float>(steps - 1));
  param_->endChangeGesture();
  repaint();
}

// --------------------------------------------------------------- GenButton

void GenButton::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(3.0f);
  const bool on = proc_.apvts.getRawParameterValue(pid::id(pid::GEN_ON))->load() > 0.5f;
  g.setColour(juce::Colours::black.withAlpha(0.3f));
  g.fillRoundedRectangle(r.translated(0, down_ ? 1.0f : 2.5f), 6.0f);
  juce::ColourGradient body(down_ ? juce::Colour(0xff1d1d1f) : juce::Colour(0xff434347), r.getX(), r.getY(),
                            juce::Colour(0xff17171a), r.getX(), r.getBottom(), false);
  g.setGradientFill(body);
  g.fillRoundedRectangle(r, 6.0f);
  g.setColour(juce::Colour(0xff0a0a0b));
  g.drawRoundedRectangle(r, 6.0f, 1.0f);
  drawLed(g, {r.getX() + 13.0f, r.getCentreY()}, 4.2f, col::amber, on ? 1.0f : 0.0f);
  g.setColour(juce::Colours::white.withAlpha(0.92f));
  g.setFont(labelFont(15.0f));
  g.drawText("GEN", r.withTrimmedLeft(22.0f).withTrimmedBottom(10.0f), juce::Justification::centred);
  g.setFont(labelFont(8.5f, false));
  g.setColour(juce::Colours::white.withAlpha(0.6f));
  g.drawText(on ? "< ocultar" : "< generar", r.withTrimmedLeft(22.0f).withTrimmedTop(22.0f),
             juce::Justification::centred);
}

void GenButton::mouseDown(const juce::MouseEvent&) {
  down_ = true;
  auto* prm = proc_.apvts.getParameter(pid::id(pid::GEN_ON));
  const bool on = prm->getValue() > 0.5f;
  prm->beginChangeGesture();
  prm->setValueNotifyingHost(on ? 0.0f : 1.0f);
  prm->endChangeGesture();
  if (!on) proc_.ensureGenCables();
  repaint();
}

// ----------------------------------------------------------------- MenuBox

MenuBox::MenuBox(juce::RangedAudioParameter* p, TextFn caption, ListFn options, juce::Colour led)
    : param_(p), caption_(std::move(caption)), options_(std::move(options)), led_(led) {
  setTooltip(p->getName(64) + "  (clic: lista)");
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void MenuBox::paint(juce::Graphics& g) {
  const int v = juce::roundToInt(param_->convertFrom0to1(param_->getValue()));
  const auto opts = options_();
  g.setColour(col::ink);
  g.setFont(labelFont(10.0f));
  g.drawText(caption_(), 0, 0, getWidth(), 12, juce::Justification::left);
  drawButtonCap(g, {10.0f, 23.0f}, 7.5f, false);
  drawLed(g, {29.0f, 23.0f}, 3.6f, led_, 1.0f);
  g.setColour(col::inkSoft);
  g.setFont(labelFont(9.5f, false));
  const juce::String txt = opts.isEmpty() ? juce::String() : opts[v % opts.size()];
  g.drawFittedText(txt, 0, 33, getWidth(), 12, juce::Justification::left, 1, 0.8f);
}

void MenuBox::mouseDown(const juce::MouseEvent&) {
  const int cur = juce::roundToInt(param_->convertFrom0to1(param_->getValue()));
  const auto opts = options_();
  juce::PopupMenu m;
  for (int i = 0; i < opts.size(); ++i)
    m.addItem(opts[i], true, cur % opts.size() == i, [this, i] {
      param_->beginChangeGesture();
      param_->setValueNotifyingHost(param_->convertTo0to1(static_cast<float>(i)));
      param_->endChangeGesture();
      repaint();
    });
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
}

// ---------------------------------------------------------------- ScaleBox

ScaleBox::ScaleBox(ResonatorIntoTextureProcessor& p) : proc_(p) {
  setTooltip("Escala de cuantización del random sampler (clic: lista)");
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ScaleBox::paint(juce::Graphics& g) {
  auto* prm = proc_.apvts.getParameter(pid::id(pid::M_SCALE));
  const int v = juce::roundToInt(prm->convertFrom0to1(prm->getValue()));
  const bool midi = v >= scales::midiIndex();
  g.setColour(col::ink);
  g.setFont(labelFont(10.0f));
  g.drawText("SCALE", 0, 0, getWidth(), 12, juce::Justification::left);
  drawButtonCap(g, {10.0f, 23.0f}, 7.5f, false);
  drawLed(g, {29.0f, 23.0f}, 3.6f, midi ? col::blue : col::green, 1.0f);
  g.setColour(col::inkSoft);
  g.setFont(labelFont(9.5f, false));
  g.drawFittedText(prm->getCurrentValueAsText(), 0, 33, getWidth(), 12, juce::Justification::left, 1, 0.8f);
}

void ScaleBox::select(int index) {
  auto* prm = proc_.apvts.getParameter(pid::id(pid::M_SCALE));
  prm->beginChangeGesture();
  prm->setValueNotifyingHost(prm->convertTo0to1(static_cast<float>(index)));
  prm->endChangeGesture();
  repaint();
}

void ScaleBox::mouseDown(const juce::MouseEvent&) {
  auto* prm = proc_.apvts.getParameter(pid::id(pid::M_SCALE));
  const int cur = juce::roundToInt(prm->convertFrom0to1(prm->getValue()));
  juce::PopupMenu m;
  juce::String group;
  juce::PopupMenu sub;
  const auto& list = scales::all();
  for (int i = 0; i <= static_cast<int>(list.size()); ++i) {
    const bool end = i == static_cast<int>(list.size());
    const juce::String g = end ? juce::String() : juce::String(list[static_cast<size_t>(i)].group);
    if ((end || g != group) && group.isNotEmpty()) {
      m.addSubMenu(group, sub);
      sub = juce::PopupMenu();
    }
    if (end) break;
    group = g;
    sub.addItem(list[static_cast<size_t>(i)].name, true, i == cur, [this, i] { select(i); });
  }
  m.addSeparator();
  m.addItem("MIDI (aprendida del teclado)", true, cur == scales::midiIndex(),
            [this] { select(scales::midiIndex()); });
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
}

// ---------------------------------------------------------------- PatchBox

void PatchBox::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  g.setColour(col::plate);
  g.fillRoundedRectangle(r, 3.0f);
  g.setColour(proc_.isUserPreset() ? juce::Colour(0xffffc457) : juce::Colour(0xff57c7ff));
  g.setFont(labelFont(9.5f));
  g.drawFittedText(proc_.currentPatchName(), r.reduced(14.0f, 0.0f).toNearestInt(),
                   juce::Justification::centred, 1, 0.8f);
  g.setColour(juce::Colours::white.withAlpha(0.6f));
  g.drawText("<", r.withWidth(14.0f), juce::Justification::centred);
  g.drawText(">", r.withTrimmedLeft(r.getWidth() - 14.0f), juce::Justification::centred);
}

void PatchBox::select(int index) {
  proc_.setCurrentProgram(index);
  repaint();
}

void PatchBox::mouseDown(const juce::MouseEvent& e) {
  const int n = proc_.getNumPrograms();
  const int cur = proc_.getCurrentProgram();
  const auto user = ResonatorIntoTextureProcessor::userPresets();
  // < > recorren fábrica y usuario seguidos
  if (e.x < 14 || e.x > getWidth() - 14) {
    const int dir = e.x < 14 ? -1 : 1;
    const int total = n + user.size();
    int pos = cur;
    if (proc_.isUserPreset())
      for (int i = 0; i < user.size(); ++i)
        if (user[i].getFileNameWithoutExtension() == proc_.currentPatchName()) pos = n + i;
    pos = (pos + dir + total) % total;
    if (pos < n) select(pos);
    else { proc_.loadUserPreset(user[pos - n]); repaint(); }
    return;
  }
  juce::PopupMenu factory, mine;
  for (int i = 0; i < n; ++i)
    factory.addItem(proc_.getProgramName(i), true, !proc_.isUserPreset() && i == cur, [this, i] { select(i); });
  for (const auto& f : user)
    mine.addItem(f.getFileNameWithoutExtension(), true,
                 proc_.isUserPreset() && f.getFileNameWithoutExtension() == proc_.currentPatchName(),
                 [this, f] { proc_.loadUserPreset(f); repaint(); });
  if (user.isEmpty()) mine.addItem("(vacío)", false, false, [] {});

  juce::PopupMenu m;
  m.addSubMenu("Fábrica", factory);
  m.addSubMenu("Usuario", mine);
  m.addSeparator();
  if (proc_.isUserPreset())
    m.addItem("Guardar \"" + proc_.currentPatchName() + "\"", [this] {
      proc_.saveUserPreset(ResonatorIntoTextureProcessor::userPresetDir()
                               .getChildFile(proc_.currentPatchName() + ".ritpreset"),
                           proc_.currentPatchName());
    });
  m.addItem("Guardar como...", [this] {
    auto* w = new juce::AlertWindow("Guardar preset", "Nombre del preset:", juce::MessageBoxIconType::NoIcon, this);
    w->addTextEditor("name", proc_.isUserPreset() ? proc_.currentPatchName() : juce::String("Mi patch"));
    w->addButton("Guardar", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancelar", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    juce::Component::SafePointer<PatchBox> safe(this);
    w->enterModalState(true, juce::ModalCallbackFunction::create([safe, w](int result) {
      if (safe == nullptr || result != 1) return;
      auto name = juce::File::createLegalFileName(w->getTextEditorContents("name").trim());
      if (name.isEmpty()) return;
      safe->proc_.saveUserPreset(ResonatorIntoTextureProcessor::userPresetDir().getChildFile(name + ".ritpreset"), name);
      safe->repaint();
    }), true);
  });
  m.addItem("Cargar archivo...", [this] {
    chooser_ = std::make_unique<juce::FileChooser>("Cargar preset", ResonatorIntoTextureProcessor::userPresetDir(), "*.ritpreset");
    juce::Component::SafePointer<PatchBox> safe(this);
    chooser_->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [safe](const juce::FileChooser& fc) {
      if (safe == nullptr || fc.getResult() == juce::File()) return;
      safe->proc_.loadUserPreset(fc.getResult());
      safe->repaint();
    });
  });
  m.addItem("Exportar a archivo...", [this] {
    chooser_ = std::make_unique<juce::FileChooser>("Exportar preset",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile(proc_.currentPatchName().trim() + ".ritpreset"),
        "*.ritpreset");
    juce::Component::SafePointer<PatchBox> safe(this);
    chooser_->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe](const juce::FileChooser& fc) {
      if (safe == nullptr || fc.getResult() == juce::File()) return;
      auto f = fc.getResult().withFileExtension(".ritpreset");
      safe->proc_.saveUserPreset(f, f.getFileNameWithoutExtension());
      safe->repaint();
    });
  });
  if (proc_.isUserPreset())
    m.addItem("Borrar \"" + proc_.currentPatchName() + "\"", [this] {
      ResonatorIntoTextureProcessor::userPresetDir().getChildFile(proc_.currentPatchName() + ".ritpreset").deleteFile();
      select(0);
    });
  m.addItem("Abrir carpeta de presets", [] { ResonatorIntoTextureProcessor::userPresetDir().startAsProcess(); });
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
}

// -------------------------------------------------------------------- Panel

Panel::Panel(ResonatorIntoTextureEditor& ed, juce::String title, juce::String subtitle, int hp)
    : ed_(ed), proc_(ed.proc()), title_(std::move(title)), subtitle_(std::move(subtitle)) {
  setSize(static_cast<int>(static_cast<float>(hp) * kHpPx), kHeight);
  setOpaque(true);
}

Knob* Panel::knob(const char* id, KnobStyle st, const juce::String& cap, float cx, float cy) {
  const int d = st == KNOB_MINI ? 26 : (st == KNOB_SMALL ? 34 : (st == KNOB_MEDIUM ? 54 : 74));
  auto* k = own(new Knob(proc_.apvts, id, st, cap));
  k->setBounds(juce::roundToInt(cx - d * 0.5f), juce::roundToInt(cy - d * 0.5f), d, d);
  return k;
}

Knob* Panel::atten(int inPort, float cx, float cy) {
  return knob(pid::id(pid::attParamForPort(inPort)), KNOB_MINI, {}, cx, cy);
}

Jack* Panel::jackIn(int in, float cx, float cy) {
  auto* j = own(new Jack(ed_, in, false));
  j->setTopLeftPosition(juce::roundToInt(cx - Jack::kW * 0.5f), juce::roundToInt(cy - Jack::kJackY));
  ed_.registerJack(j);
  return j;
}

Jack* Panel::jackOut(int out, float cx, float cy) {
  auto* j = own(new Jack(ed_, out, true));
  j->setTopLeftPosition(juce::roundToInt(cx - Jack::kW * 0.5f), juce::roundToInt(cy - Jack::kJackY));
  ed_.registerJack(j);
  return j;
}

void Panel::caption(juce::Graphics& g, const juce::String& text, float cx, float y, float size) {
  g.setColour(col::ink);
  g.setFont(labelFont(size));
  g.drawText(text, juce::Rectangle<float>(cx - 70.0f, y, 140.0f, size + 3.0f), juce::Justification::centred);
}

void Panel::groupBox(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& label) {
  g.setColour(col::line);
  g.drawRoundedRectangle(r, 5.0f, 1.0f);
  if (label.isNotEmpty()) {
    g.setFont(labelFont(9.0f));
    const float tw = juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), label) + 10.0f;
    auto lr = juce::Rectangle<float>(r.getX() + 10.0f, r.getY() - 6.0f, tw, 12.0f);
    juce::ColourGradient bg(col::panelTop, 0, 0, col::panelBottom, 0, static_cast<float>(kHeight), false);
    g.setGradientFill(bg);
    g.fillRect(lr);
    g.setColour(col::inkSoft);
    g.drawText(label, lr, juce::Justification::centred);
  }
}

void Panel::outPlate(juce::Graphics& g, juce::Rectangle<float> r) {
  g.setColour(col::plate);
  g.fillRoundedRectangle(r, 5.0f);
  g.setColour(juce::Colours::black.withAlpha(0.5f));
  g.drawRoundedRectangle(r, 5.0f, 1.0f);
}

void Panel::paint(juce::Graphics& g) {
  const float scale = g.getInternalContext().getPhysicalPixelScaleFactor();
  const int w = getWidth(), h = getHeight();
  const int iw = juce::roundToInt(static_cast<float>(w) * scale);
  if (background_.isNull() || background_.getWidth() != iw) {
    background_ = juce::Image(juce::Image::ARGB, iw, juce::roundToInt(static_cast<float>(h) * scale), true);
    juce::Graphics bg(background_);
    bg.addTransform(juce::AffineTransform::scale(scale));
    juce::ColourGradient grad(col::panelTop, 0, 0, col::panelBottom, 0, static_cast<float>(h), false);
    bg.setGradientFill(grad);
    bg.fillRect(0, 0, w, h);
    juce::Random rnd(static_cast<juce::int64>(title_.hashCode()));
    for (int y = 0; y < h; ++y) {
      const float a = rnd.nextFloat() * 0.07f;
      bg.setColour((rnd.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha(a));
      bg.drawHorizontalLine(y, 0.0f, static_cast<float>(w));
    }
    bg.setColour(juce::Colours::white.withAlpha(0.55f));
    bg.drawVerticalLine(0, 0.0f, static_cast<float>(h));
    bg.drawHorizontalLine(0, 0.0f, static_cast<float>(w));
    bg.setColour(juce::Colours::black.withAlpha(0.35f));
    bg.drawVerticalLine(w - 1, 0.0f, static_cast<float>(h));
    bg.drawHorizontalLine(h - 1, 0.0f, static_cast<float>(w));
    auto hole = [&](float cx, float cy) {
      bg.setColour(juce::Colour(0xff7d7e82));
      bg.fillRoundedRectangle(cx - 6.5f, cy - 4.0f, 13.0f, 8.0f, 4.0f);
      bg.setColour(juce::Colour(0xff26262a));
      bg.fillRoundedRectangle(cx - 5.5f, cy - 3.0f, 11.0f, 6.0f, 3.0f);
    };
    hole(16.0f, 9.0f);
    hole(16.0f, static_cast<float>(h) - 9.0f);
    if (w > 170) {
      hole(static_cast<float>(w) - 16.0f, 9.0f);
      hole(static_cast<float>(w) - 16.0f, static_cast<float>(h) - 9.0f);
    }
    bg.setColour(col::ink);
    bg.setFont(labelFont(19.0f));
    bg.drawText(title_, 0, 20, w, 22, juce::Justification::centred);
    bg.setColour(col::inkSoft);
    bg.setFont(labelFont(9.5f, false));
    bg.drawText(subtitle_, 0, 41, w, 12, juce::Justification::centred);
    paintStatic(bg);  // placas y cajas por debajo de las leyendas
    for (auto* c : owned_) {
      if (auto* k = dynamic_cast<Knob*>(c)) {
        if (k->caption.isEmpty() || dynamicCaptions_) continue;
        const auto b = k->getBounds().toFloat();
        const float size = k->style == KNOB_MINI ? 7.5f : (k->style == KNOB_SMALL ? 9.0f : 10.5f);
        caption(bg, k->caption, b.getCentreX(), b.getY() - size - 1.0f, size);
      }
    }
  }
  g.drawImage(background_, getLocalBounds().toFloat());
  paintDynamic(g);
}

// ------------------------------------------------------------------ IoPanel

IoPanel::IoPanel(ResonatorIntoTextureEditor& ed) : Panel(ed, "in / out", "ResonatorIntoTexture", 6) {
  own(new PatchBox(proc_))->setBounds(6, 58, 114, 18);
  knob(pid::id(pid::IN_GAIN), KNOB_MEDIUM, "INPUT", 63, 128);
  own(new GenButton(proc_))->setBounds(21, 202, 84, 52);
  knob(pid::id(pid::MIX), KNOB_MEDIUM, "MIX", 63, 358);
  knob(pid::id(pid::OUT_GAIN), KNOB_MEDIUM, "OUTPUT", 63, 444);
  atten(IN_IO_MIX, 34, 508);
  jackIn(IN_IO_MIX, 88, 508);
  atten(IN_IO_OUT_L, 38, 638)->caption = "LEVEL";
  atten(IN_IO_OUT_R, 88, 638)->caption = "LEVEL";
  jackIn(IN_IO_OUT_L, 38, 688);
  jackIn(IN_IO_OUT_R, 88, 688);
}

void IoPanel::mouseDown(const juce::MouseEvent& e) {
  if (!e.mods.isPopupMenu() && e.y > 60) return;
  juce::PopupMenu m;
  m.addSectionHeader("Zoom");
  const float current = static_cast<float>(static_cast<double>(proc_.apvts.state.getProperty("zoom", 1.0)));
  for (float z : {0.6f, 0.75f, 0.9f, 1.0f, 1.25f, 1.5f})
    m.addItem(juce::String(juce::roundToInt(z * 100.0f)) + " %", true, std::abs(z - current) < 0.01f,
              [this, z] { proc_.apvts.state.setProperty("zoom", z, nullptr); });
  m.addSeparator();
  m.addItem("Quitar todos los cables", [this] { proc_.clearCables(); });
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
}

void IoPanel::tick() {
  in_ = decay(in_, proc_.status().trackLevel.exchange(0.0f));
  const int mask = proc_.status().midiMask.load();
  if (mask != lastMask_) { midi_ = 1.0f; lastMask_ = mask; }
  midi_ *= 0.93f;
  repaint();
}

void IoPanel::paintStatic(juce::Graphics& g) {
  caption(g, "GENERATOR", 63, 188);
  groupBox(g, {14.0f, 268.0f, 98.0f, 30.0f}, "MIDI");
  groupBox(g, {10.0f, 600.0f, 106.0f, 114.0f}, "TO DAW");
}

void IoPanel::paintDynamic(juce::Graphics& g) {
  const float db = juce::Decibels::gainToDecibels(in_, -60.0f);
  for (int i = 0; i < 5; ++i) {
    const float th = -48.0f + 10.0f * static_cast<float>(i);
    const auto c = i < 3 ? col::green : (i == 3 ? col::amber : col::red);
    drawLed(g, {31.0f + 16.0f * static_cast<float>(i), 172.0f}, 3.2f, c, db > th ? 1.0f : 0.0f);
  }
  auto& s = proc_.status();
  const int mask = s.midiMask.load();
  const int root = s.midiRoot.load();
  drawLed(g, {28.0f, 283.0f}, 3.6f, col::blue, std::max(midi_, mask ? 0.25f : 0.0f));
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  g.setColour(col::ink);
  g.setFont(labelFont(10.0f));
  juce::String txt = "--";
  if (mask != 0 && root >= 0) {
    int count = 0;
    for (int i = 0; i < 12; ++i) count += (mask >> i) & 1;
    txt = juce::String(names[root]) + " / " + juce::String(count) + " notas";
  }
  g.drawText(txt, 38, 277, 72, 12, juce::Justification::left);
}

// --------------------------------------------------------------- RingsPanel

RingsPanel::RingsPanel(ResonatorIntoTextureEditor& ed)
    : Panel(ed, "resonator", "modal  -  sympathetic strings  -  string", 14) {
  knob(pid::id(pid::R_FREQ), KNOB_BIG, "FREQUENCY", 80, 132);
  knob(pid::id(pid::R_STRUCT), KNOB_BIG, "STRUCTURE", 214, 132);
  knob(pid::id(pid::R_BRIGHT), KNOB_MEDIUM, "BRIGHTNESS", 52, 240);
  knob(pid::id(pid::R_DAMP), KNOB_MEDIUM, "DAMPING", 147, 240);
  knob(pid::id(pid::R_POS), KNOB_MEDIUM, "POSITION", 242, 240);

  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::R_MODEL)), "RESONATOR", 3,
      [](int v, int led) {
        if (v < 3) return led == v ? col::green : juce::Colours::transparentBlack;
        return led == v - 3 ? col::red : juce::Colours::transparentBlack;
      }, true))->setBounds(26, 306, 130, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::R_POLY)), "POLYPHONY", 3,
      [](int v, int led) { return led == v ? col::amber : juce::Colours::transparentBlack; }, true))
      ->setBounds(172, 306, 100, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::R_EXCITER)), "EXCITER", 3,
      [](int v, int led) { return led == v ? col::green : juce::Colours::transparentBlack; }, true))
      ->setBounds(20, 372, 118, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::R_QUANT)), "NOTES", 3,
      [](int v, int led) { return led == v ? col::blue : juce::Colours::transparentBlack; }, true))
      ->setBounds(142, 372, 76, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::R_ON)), "ON", 1,
      [](int v, int) { return v ? col::green : juce::Colours::transparentBlack; }, false))
      ->setBounds(232, 372, 50, 30);

  const int cv[] = {IN_R_FM, IN_R_STRUCT, IN_R_BRIGHT, IN_R_DAMP, IN_R_POS};
  for (int i = 0; i < 5; ++i) {
    const float x = 37.0f + 55.0f * static_cast<float>(i);
    atten(cv[i], x, 474);
    jackIn(cv[i], x, 530);
  }
  jackIn(IN_R_STRUM, 37, 628);
  jackIn(IN_R_VOCT, 92, 628);
  atten(IN_R_IN, 147, 586)->caption = "LEVEL";
  jackIn(IN_R_IN, 147, 628);
  jackOut(OUT_R_ODD, 212, 628);
  jackOut(OUT_R_EVEN, 262, 628);
}

void RingsPanel::tick() {
  if (proc_.status().strum.exchange(0.0f) > 0.5f) strum_ = 1.0f;
  strum_ *= 0.86f;
  repaint();
}

void RingsPanel::paintStatic(juce::Graphics& g) {
  groupBox(g, {16.0f, 298.0f, 262.0f, 62.0f});
  caption(g, "STRUM", 256, 404, 8.5f);
  groupBox(g, {12.0f, 446.0f, 270.0f, 108.0f}, "CV");
  outPlate(g, {186.0f, 588.0f, 98.0f, 64.0f});
}

void RingsPanel::paintDynamic(juce::Graphics& g) {
  drawLed(g, {256.0f, 424.0f}, 3.6f, col::yellow, strum_);
}

// -------------------------------------------------------------- CloudsPanel

namespace {
// Rótulos de los 9 knobs según el modo (POS SIZE PITCH DENS TEX BLEND SPREAD FDBK VERB)
const char* const kTexKnob[8][9] = {
  {"POSITION", "SIZE", "PITCH", "DENSITY", "TEXTURE", "DRY/WET", "SPREAD", "FEEDBACK", "REVERB"},
  {"POSITION", "WINDOW", "PITCH", "DIFFUSION", "FILTER", "DRY/WET", "SPREAD", "FEEDBACK", "REVERB"},
  {"DELAY", "SIZE", "PITCH", "DIFFUSION", "FILTER", "DRY/WET", "SPREAD", "FEEDBACK", "REVERB"},
  {"POSITION", "SIZE", "PITCH", "QUANTIZE", "TEXTURE", "DRY/WET", "SPREAD", "FEEDBACK", "REVERB"},
  {"PRE-DELAY", "ROOM SIZE", "SHIFT", "DECAY", "FILTER", "DRY/WET", "DIFFUSION", "MOD RATE", "MOD AMT"},
  {"BURST", "CHORD", "PITCH", "FEEDBACK", "FILTER", "DISTORT", "STEREO", "HARMONIC", "V. SPREAD"},
  {"CHAR A", "CONTOUR", "TUNE", "CHAR B", "FILTER", "SEND", "PAN WIDTH", "ECHO FB", "VERB TAIL"},
  {"TIME", "REPEATS", "BEND", "BREAK", "CORRUPT", "MIX", "WIDTH", "REGEN", "REV CHANCE"},
};
}  // namespace

CloudsPanel::CloudsPanel(ResonatorIntoTextureEditor& ed)
    : Panel(ed, "texture", "Parasites + trig drums + data bender", 18) {
  dynamicCaptions_ = true;
  knobs_.add(knob(pid::id(pid::C_POS), KNOB_BIG, {}, 68, 142));
  knobs_.add(knob(pid::id(pid::C_SIZE), KNOB_BIG, {}, 189, 142));
  knobs_.add(knob(pid::id(pid::C_PITCH), KNOB_BIG, {}, 310, 142));
  knobs_.add(knob(pid::id(pid::C_DENS), KNOB_MEDIUM, {}, 92, 246));
  knobs_.add(knob(pid::id(pid::C_TEX), KNOB_MEDIUM, {}, 286, 246));
  knobs_.add(knob(pid::id(pid::C_DRYWET), KNOB_SMALL, {}, 62, 334));
  knobs_.add(knob(pid::id(pid::C_SPREAD), KNOB_SMALL, {}, 147, 334));
  knobs_.add(knob(pid::id(pid::C_FB), KNOB_SMALL, {}, 232, 334));
  knobs_.add(knob(pid::id(pid::C_REV), KNOB_SMALL, {}, 317, 334));

  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::C_FREEZE)), "FREEZE", 1,
      [](int v, int) { return v ? col::blue : juce::Colours::transparentBlack; }, false))
      ->setBounds(166, 232, 60, 30);
  own(new MenuBox(proc_.apvts.getParameter(pid::id(pid::C_MODE)), [] { return juce::String("MODE"); },
      [] { return ResonatorIntoTextureProcessor::texModeNames(); }, col::green))
      ->setBounds(20, 376, 106, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::C_QUALITY)), "QUALITY", 4,
      [](int v, int led) { return led == v ? col::red : juce::Colours::transparentBlack; }, true))
      ->setBounds(132, 376, 112, 46);
  auto* self = this;
  fx_ = own(new MenuBox(proc_.apvts.getParameter(pid::id(pid::C_FX)), [] { return juce::String("PERF FX"); },
      [self] {
        juce::StringArray a{"Off", "Stutter", "Tape stop", "Filter sweep", "Bitcrush", "Wash", "Reverse", "Gate"};
        a.add(self->mode() == ric::TEX_DRUMS ? "Echo throw" : "Octave down");
        return a;
      }, col::amber));
  fx_->setBounds(252, 376, 116, 46);
  alt1_ = own(new MenuBox(proc_.apvts.getParameter(pid::id(pid::C_ALT1)),
      [self] {
        const int m = self->mode();
        return juce::String(m == ric::TEX_DRUMS ? "VOICE (KNOBS)" : m == ric::TEX_BENDER ? "ENGINE" : "GRAINS");
      },
      [self] {
        const int m = self->mode();
        if (m == ric::TEX_DRUMS)
          return juce::StringArray{"BD kick", "SD snare", "OH open hat", "CH closed hat", "CP clap",
                                   "MT multi", "SA chord stab", "SB resonator"};
        if (m == ric::TEX_BENDER)
          return juce::StringArray{"Traverse", "Silence", "Micro", "Traverse rev.", "Silence rev.", "Micro rev."};
        return juce::StringArray{"Forward", "Reverse"};
      }, col::orange));
  alt1_->setBounds(20, 426, 170, 46);
  alt2_ = own(new MenuBox(proc_.apvts.getParameter(pid::id(pid::C_ALT2)),
      [self] { return juce::String(self->mode() == ric::TEX_DRUMS ? "KIT / AUDIO IN" : "CORRUPT"); },
      [self] {
        if (self->mode() == ric::TEX_DRUMS)
          return juce::StringArray{"Analog / Thru", "Punch / Thru", "Analog / Skin", "Punch / Skin",
                                   "Analog / Chop", "Punch / Chop"};
        return juce::StringArray{"Decimate", "Dropout", "Destroy", "DJ filter", "Vinyl"};
      }, col::orange));
  alt2_->setBounds(198, 426, 170, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::C_ON)), "ON", 1,
      [](int v, int) { return v ? col::green : juce::Colours::transparentBlack; }, false))
      ->setBounds(318, 58, 50, 30);

  const int cv[] = {IN_C_POS, IN_C_SIZE, IN_C_PITCH, IN_C_DENS, IN_C_TEX,
                    IN_C_DRYWET, IN_C_SPREAD, IN_C_FB, IN_C_REV};
  for (int i = 0; i < 9; ++i) {
    const float x = 28.0f + 40.5f * static_cast<float>(i);
    atten(cv[i], x, 512);
    jackIn(cv[i], x, 566);
  }
  jackIn(IN_C_FREEZE, 32, 660);
  jackIn(IN_C_TRIG, 80, 660);
  atten(IN_C_IN_L, 128, 616)->caption = "LEVEL";
  atten(IN_C_IN_R, 176, 616)->caption = "LEVEL";
  jackIn(IN_C_IN_L, 128, 660);
  jackIn(IN_C_IN_R, 176, 660);
  jackOut(OUT_C_OUT_L, 282, 660);
  jackOut(OUT_C_OUT_R, 334, 660);
}

int CloudsPanel::mode() const {
  return juce::roundToInt(proc_.apvts.getRawParameterValue(pid::id(pid::C_MODE))->load());
}

void CloudsPanel::tick() {
  out_ = decay(out_, std::max(ed_.outLevel(OUT_C_OUT_L), ed_.outLevel(OUT_C_OUT_R)));
  const int m = mode();
  if (m != shownMode_) {
    shownMode_ = m;
    alt1_->setVisible(m == ric::TEX_GRANULAR || m == ric::TEX_DRUMS || m == ric::TEX_BENDER);
    alt2_->setVisible(m == ric::TEX_DRUMS || m == ric::TEX_BENDER);
    fx_->setVisible(m == ric::TEX_DRUMS || m == ric::TEX_BENDER);
    for (int i = 0; i < knobs_.size(); ++i) knobs_[i]->caption = kTexKnob[m][i];
  }
  repaint();
}

void CloudsPanel::paintStatic(juce::Graphics& g) {
  groupBox(g, {18.0f, 302.0f, 342.0f, 62.0f}, "BLEND");
  groupBox(g, {8.0f, 484.0f, 362.0f, 108.0f}, "CV");
  outPlate(g, {254.0f, 620.0f, 108.0f, 64.0f});
  caption(g, "LEVEL", 128, 594, 7.5f);
  caption(g, "LEVEL", 176, 594, 7.5f);
}

void CloudsPanel::paintDynamic(juce::Graphics& g) {
  const int m = mode();
  // Rótulos dinámicos de los knobs
  for (auto* k : knobs_) {
    const auto b = k->getBounds().toFloat();
    const float size = k->style == KNOB_SMALL ? 9.0f : 10.5f;
    caption(g, k->caption, b.getCentreX(), b.getY() - size - 1.0f, size);
  }
  if (m == ric::TEX_DRUMS) {
    // Un LED por voz, en el orden de los jacks
    static const char* v[8] = {"BD", "SD", "OH", "CH", "CP", "MT", "SA", "SB"};
    for (int i = 0; i < 8; ++i) {
      const float x = 116.0f + 21.0f * static_cast<float>(i);
      drawLed(g, {x, 66.0f}, 3.2f, i % 2 ? col::green : col::red, proc_.status().drumHit[i].load() * 1.5f);
      g.setColour(col::inkSoft);
      g.setFont(labelFont(7.5f));
      g.drawText(v[i], juce::Rectangle<float>(x - 10.0f, 73.0f, 20.0f, 9.0f), juce::Justification::centred);
    }
  } else {
    const float db = juce::Decibels::gainToDecibels(out_, -60.0f);
    for (int i = 0; i < 4; ++i) {
      const float th = -42.0f + 10.0f * static_cast<float>(i);
      drawLed(g, {165.0f + 16.0f * static_cast<float>(i), 66.0f}, 3.4f, i < 3 ? col::green : col::red,
              db > th ? 1.0f : 0.0f);
    }
  }
  if (m == ric::TEX_DRUMS || m == ric::TEX_BENDER) {
    g.setColour(col::inkSoft);
    g.setFont(labelFont(8.5f, false));
    g.drawText(m == ric::TEX_DRUMS ? "FREEZE = DUB" : "", 160, 264, 80, 10, juce::Justification::centred);
  }
  if (proc_.status().texBusy.load()) {
    g.setColour(juce::Colour(0xddb02020));
    g.fillRoundedRectangle(84.0f, 84.0f, 210.0f, 18.0f, 4.0f);
    g.setColour(juce::Colours::white);
    g.setFont(labelFont(9.5f));
    g.drawText("en uso por otra instancia del plugin", 84, 84, 210, 18, juce::Justification::centred);
  }
}

// ------------------------------------------------------------- MarblesPanel

MarblesPanel::MarblesPanel(ResonatorIntoTextureEditor& ed)
    : Panel(ed, "random sampler", "t  -  deja vu  -  x", 18) {
  knob(pid::id(pid::M_RATE), KNOB_BIG, "RATE", 66, 132);
  knob(pid::id(pid::M_DV), KNOB_DEJAVU, "DEJA VU", 189, 132);
  knob(pid::id(pid::M_SPREAD), KNOB_BIG, "SPREAD", 312, 132);
  knob(pid::id(pid::M_BIAS), KNOB_MEDIUM, "BIAS", 50, 240);
  knob(pid::id(pid::M_LEN), KNOB_SMALL, "LENGTH", 189, 256);
  knob(pid::id(pid::M_XBIAS), KNOB_MEDIUM, "BIAS", 328, 240);
  knob(pid::id(pid::M_JITTER), KNOB_MEDIUM, "JITTER", 50, 326);
  knob(pid::id(pid::M_STEPS), KNOB_MEDIUM, "STEPS", 328, 326);
  knob(pid::id(pid::M_ROOT), KNOB_SMALL, "ROOT", 36, 448);

  auto onOff = [](juce::Colour c) {
    return [c](int v, int) { return v ? c : juce::Colours::transparentBlack; };
  };
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_DVT)), "T", 1, onOff(col::orange), false))
      ->setBounds(122, 186, 40, 30);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_DVX)), "X", 1, onOff(col::orange), false))
      ->setBounds(222, 186, 40, 30);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_TMODEL)), "T MODE", 3,
      [](int v, int led) {
        if (v < 3) return led == v ? col::green : juce::Colours::transparentBlack;
        if (v < 6) return led == v - 3 ? col::amber : juce::Colours::transparentBlack;
        return col::red;
      }, true))->setBounds(14, 374, 92, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_TRANGE)), "T RANGE", 3,
      [](int v, int led) { return led == v ? col::green : juce::Colours::transparentBlack; }, true))
      ->setBounds(110, 374, 76, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_XRANGE)), "X RANGE", 3,
      [](int v, int led) { return led == v ? col::amber : juce::Colours::transparentBlack; }, true))
      ->setBounds(196, 374, 76, 46);
  own(new ScaleBox(proc_))->setBounds(284, 374, 90, 46);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_SYNC)), "SYNC", 1,
      [](int v, int) { return v ? col::blue : juce::Colours::transparentBlack; }, false))
      ->setBounds(136, 292, 50, 30);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::M_SYNC_DIV)), "DIV", 0, nullptr, true))
      ->setBounds(196, 292, 54, 46);

  jackIn(IN_M_TCLK, 28, 566);
  const int cv[] = {IN_M_RATE, IN_M_BIAS, IN_M_JITTER, IN_M_DV, IN_M_LEN, IN_M_SPREAD, IN_M_XBIAS, IN_M_STEPS};
  for (int i = 0; i < 8; ++i) {
    const float x = 68.5f + 40.5f * static_cast<float>(i);
    atten(cv[i], x, 512);
    jackIn(cv[i], x, 566);
  }
  const int outs[] = {OUT_M_T1, OUT_M_T2, OUT_M_T3, OUT_M_Y, OUT_M_X1, OUT_M_X2, OUT_M_X3};
  for (int i = 0; i < 7; ++i) jackOut(outs[i], 34.0f + 51.7f * static_cast<float>(i), 660);
}

void MarblesPanel::paintStatic(juce::Graphics& g) {
  groupBox(g, {12.0f, 62.0f, 112.0f, 300.0f}, "T");
  groupBox(g, {254.0f, 62.0f, 112.0f, 300.0f}, "X");
  groupBox(g, {72.0f, 434.0f, 296.0f, 30.0f}, "NOTES");
  groupBox(g, {8.0f, 484.0f, 362.0f, 108.0f}, "CV");
  outPlate(g, {10.0f, 620.0f, 358.0f, 64.0f});
}

void MarblesPanel::paintDynamic(juce::Graphics& g) {
  const auto& ap = proc_.apvts;
  const int scale = juce::roundToInt(ap.getRawParameterValue(pid::id(pid::M_SCALE))->load());
  int root = juce::roundToInt(ap.getRawParameterValue(pid::id(pid::M_ROOT))->load());
  int mask = scales::mask(scale);
  const bool midiScale = scale >= scales::midiIndex();
  if (midiScale) {
    const int m = proc_.status().midiMask.load();
    const int r = proc_.status().midiRoot.load();
    if (m != 0 && r >= 0) {
      root = r;
      mask = 0;
      for (int i = 0; i < 12; ++i) if (m & (1 << ((r + i) % 12))) mask |= 1 << i;
    }
  }
  for (int i = 0; i < 12; ++i) {
    const int pc = (root + i) % 12;
    const float x = 84.0f + 23.5f * static_cast<float>(pc);
    const bool black = (0x54a >> pc) & 1;
    g.setColour(black ? juce::Colour(0xff2c2c2f) : juce::Colour(0xfff4f3ef));
    g.fillRoundedRectangle(x, 441.0f, 18.0f, 16.0f, 2.5f);
    g.setColour(juce::Colour(0xff7b7b7f));
    g.drawRoundedRectangle(x, 441.0f, 18.0f, 16.0f, 2.5f, 0.6f);
    if (mask & (1 << i))
      drawLed(g, {x + 9.0f, 449.0f}, 3.0f, i == 0 ? col::orange : (midiScale ? col::blue : col::green), 1.0f);
  }
}

// -------------------------------------------------------------- PlaitsPanel

PlaitsPanel::PlaitsPanel(ResonatorIntoTextureEditor& ed)
    : Panel(ed, "macro oscillator", "24 models", 12) {
  auto* eng = proc_.apvts.getParameter(pid::id(pid::P_ENGINE));
  own(new StepButton(eng, -1))->setBounds(14, 64, 26, 26);
  own(new StepButton(eng, +1))->setBounds(212, 64, 26, 26);
  knob(pid::id(pid::P_PITCH), KNOB_BIG, "FREQUENCY", 68, 170);
  knob(pid::id(pid::P_HARM), KNOB_BIG, "HARMONICS", 184, 170);
  knob(pid::id(pid::P_TIMBRE), KNOB_MEDIUM, "TIMBRE", 68, 268);
  knob(pid::id(pid::P_MORPH), KNOB_MEDIUM, "MORPH", 184, 268);
  knob(pid::id(pid::P_DECAY), KNOB_SMALL, "DECAY", 48, 344);
  knob(pid::id(pid::P_LPG), KNOB_SMALL, "LPG COLOUR", 126, 344);
  knob(pid::id(pid::P_LEVEL), KNOB_SMALL, "LEVEL", 204, 344);
  own(new CycleButton(proc_.apvts.getParameter(pid::id(pid::P_QUANT)), "NOTES", 3,
      [](int v, int led) { return led == v ? col::blue : juce::Colours::transparentBlack; }, true))
      ->setBounds(28, 390, 200, 46);

  const int row1[] = {IN_P_FM, IN_P_HARM, IN_P_TIMBRE, IN_P_MORPH};
  const int row2[] = {IN_P_DECAY, IN_P_LPG, IN_P_LEVEL, IN_P_MODEL};
  for (int i = 0; i < 4; ++i) {
    const float x = 36.0f + 60.0f * static_cast<float>(i);
    atten(row1[i], x - 15.0f, 486);
    jackIn(row1[i], x + 9.0f, 492);
    atten(row2[i], x - 15.0f, 552);
    jackIn(row2[i], x + 9.0f, 558);
  }
  jackIn(IN_P_VOCT, 36, 660);
  jackIn(IN_P_TRIG, 90, 660);
  jackOut(OUT_P_OUT, 160, 660);
  jackOut(OUT_P_AUX, 214, 660);
}

void PlaitsPanel::paintStatic(juce::Graphics& g) {
  caption(g, "MODEL", 126, 58, 9.5f);
  groupBox(g, {18.0f, 382.0f, 216.0f, 58.0f});
  groupBox(g, {8.0f, 456.0f, 236.0f, 132.0f}, "CV");
  outPlate(g, {130.0f, 620.0f, 112.0f, 64.0f});
}

void PlaitsPanel::paintDynamic(juce::Graphics& g) {
  const int e = juce::roundToInt(proc_.apvts.getRawParameterValue(pid::id(pid::P_ENGINE))->load());
  const int bank = e / 8;
  const auto bankCol = bank == 0 ? col::orange : (bank == 1 ? col::green : col::red);
  for (int i = 0; i < 8; ++i)
    drawLed(g, {58.0f + 19.4f * static_cast<float>(i), 77.0f}, 3.6f, bankCol, i == e % 8 ? 1.0f : 0.0f);
  g.setColour(col::ink);
  g.setFont(labelFont(11.0f));
  g.drawText(ResonatorIntoTextureProcessor::plaitsEngineNames()[e], 0, 94, getWidth(), 14,
             juce::Justification::centred);
}

// --------------------------------------------------------------- CableLayer

static void drawCable(juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour c,
                      float alpha) {
  const float dist = a.getDistanceFrom(b);
  const float sag = 24.0f + dist * 0.22f;
  const juce::Point<float> mid((a.x + b.x) * 0.5f,
                              std::min(std::max(a.y, b.y) + sag, static_cast<float>(Panel::kHeight) + 40.0f));
  juce::Path p;
  p.startNewSubPath(a);
  p.quadraticTo(mid, b);
  g.setColour(juce::Colours::black.withAlpha(0.28f * alpha));
  g.strokePath(p, juce::PathStrokeType(6.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
               juce::AffineTransform::translation(0.0f, 2.0f));
  g.setColour(c.darker(0.45f).withAlpha(alpha));
  g.strokePath(p, juce::PathStrokeType(5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  g.setColour(c.withAlpha(alpha));
  g.strokePath(p, juce::PathStrokeType(3.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  g.setColour(c.brighter(0.5f).withAlpha(0.5f * alpha));
  g.strokePath(p, juce::PathStrokeType(1.0f), juce::AffineTransform::translation(-0.8f, -0.8f));
  for (auto pt : {a, b}) {
    g.setColour(c.darker(0.5f).withAlpha(alpha));
    g.fillEllipse(pt.x - 6.0f, pt.y - 6.0f, 12.0f, 12.0f);
    g.setColour(c.withAlpha(alpha));
    g.fillEllipse(pt.x - 4.6f, pt.y - 4.6f, 9.2f, 9.2f);
    g.setColour(juce::Colours::black.withAlpha(0.5f * alpha));
    g.fillEllipse(pt.x - 1.8f, pt.y - 1.8f, 3.6f, 3.6f);
  }
}

void CableLayer::paint(juce::Graphics& g) {
  auto& content = ed_.content();
  for (const auto& c : ed_.cables()) {
    auto* jo = ed_.jackFor(c.out, true);
    auto* ji = ed_.jackFor(c.in, false);
    if (!jo || !ji || !ed_.jackVisible(jo) || !ed_.jackVisible(ji)) continue;
    drawCable(g, jo->centreIn(content), ji->centreIn(content), cableColour(c.colour), 0.82f);
  }
  if (ed_.dragging()) {
    const auto a = ed_.dragAnchor()->centreIn(content);
    drawCable(g, a, ed_.dragPoint(), cableColour(ed_.dragColour()), 0.9f);
  }
}

}  // namespace ui

// =================================================================== Editor

ResonatorIntoTextureEditor::ResonatorIntoTextureEditor(ResonatorIntoTextureProcessor& p)
    : AudioProcessorEditor(&p), proc_(p), marbles_(*this), plaits_(*this), io_(*this), rings_(*this),
      clouds_(*this), cableLayer_(*this) {
  setLookAndFeel(&lnf_);
  addAndMakeVisible(content_);
  content_.addChildComponent(marbles_);
  content_.addChildComponent(plaits_);
  content_.addAndMakeVisible(io_);
  content_.addAndMakeVisible(rings_);
  content_.addAndMakeVisible(clouds_);
  content_.addAndMakeVisible(cableLayer_);
  zoom_ = static_cast<float>(static_cast<double>(p.apvts.state.getProperty("zoom", 1.0)));
  genShown_ = p.apvts.getRawParameterValue(pid::id(pid::GEN_ON))->load() > 0.5f;
  cablesCache_ = proc_.getCables();
  cablesSeen_ = proc_.cablesVersion();
  updateLayout();
  startTimerHz(30);
}

ResonatorIntoTextureEditor::~ResonatorIntoTextureEditor() {
  stopTimer();
  setLookAndFeel(nullptr);
}

void ResonatorIntoTextureEditor::registerJack(ui::Jack* j) {
  if (j->isOutput()) outJacks_[j->index()] = j;
  else inJacks_[j->index()] = j;
}

int ResonatorIntoTextureEditor::textureMode() const {
  return juce::roundToInt(proc_.apvts.getRawParameterValue(pid::id(pid::C_MODE))->load());
}

juce::String ResonatorIntoTextureEditor::jackLabel(int index, bool isOutput) const {
  if (isOutput) return outInfo(index).label;
  if (textureMode() == ric::TEX_DRUMS) {
    switch (index) {
      case IN_C_POS: return "BD";
      case IN_C_SIZE: return "SD";
      case IN_C_PITCH: return "OH";
      case IN_C_TRIG: return "CH";
      case IN_C_DENS: return "CP";
      case IN_C_TEX: return "MT";
      case IN_C_DRYWET: return "SA";
      case IN_C_FREEZE: return "SB";
      default: break;
    }
  }
  if (textureMode() == ric::TEX_BENDER) {
    if (index == IN_C_TRIG) return "CLOCK";
    if (index == IN_C_PITCH) return "V/OCT";
  }
  return inInfo(index).label;
}

bool ResonatorIntoTextureEditor::jackVisible(const ui::Jack* j) const {
  const auto m = j->isOutput() ? outInfo(j->index()).module : inInfo(j->index()).module;
  return genShown_ || (m != MOD_MARBLES && m != MOD_PLAITS);
}

float ResonatorIntoTextureEditor::inLevel(int in) const {
  float l = 0.0f;
  for (const auto& c : cablesCache_)
    if (c.in == in) l = std::max(l, levels_[c.out]);
  return l;
}

void ResonatorIntoTextureEditor::updateLayout() {
  int x = 0;
  marbles_.setVisible(genShown_);
  plaits_.setVisible(genShown_);
  if (genShown_) {
    marbles_.setTopLeftPosition(x, 0); x += marbles_.getWidth();
    plaits_.setTopLeftPosition(x, 0); x += plaits_.getWidth();
  }
  io_.setTopLeftPosition(x, 0); x += io_.getWidth();
  rings_.setTopLeftPosition(x, 0); x += rings_.getWidth();
  clouds_.setTopLeftPosition(x, 0); x += clouds_.getWidth();
  content_.setBounds(0, 0, x, ui::Panel::kHeight);
  cableLayer_.setBounds(content_.getLocalBounds());
  cableLayer_.toFront(false);
  content_.setTransform(juce::AffineTransform::scale(zoom_));
  setSize(juce::roundToInt(static_cast<float>(x) * zoom_),
          juce::roundToInt(static_cast<float>(ui::Panel::kHeight) * zoom_));
}

void ResonatorIntoTextureEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff202022)); }

// ------------------------------------------------------------ arrastre

void ResonatorIntoTextureEditor::beginDrag(ui::Jack& j, const juce::MouseEvent& e) {
  cablesSeen_ = proc_.cablesVersion();
  cablesCache_ = proc_.getCables();
  dragAnchor_ = &j;
  dragColour_ = static_cast<int>(cablesCache_.size()) + 1;
  if (!j.isOutput()) {
    // Si la entrada tiene cables, se "despega" el último y se arrastra su extremo.
    for (auto it = cablesCache_.rbegin(); it != cablesCache_.rend(); ++it) {
      if (it->in == j.index()) {
        dragAnchor_ = outJacks_[it->out];
        dragColour_ = it->colour;
        proc_.removeCable(it->out, it->in);
        break;
      }
    }
  }
  dragPoint_ = e.getEventRelativeTo(&content_).position;
  cableLayer_.repaint();
}

void ResonatorIntoTextureEditor::dragTo(const juce::MouseEvent& e) {
  if (!dragAnchor_) return;
  dragPoint_ = e.getEventRelativeTo(&content_).position;
  cableLayer_.repaint();
}

void ResonatorIntoTextureEditor::endDrag(const juce::MouseEvent& e) {
  if (!dragAnchor_) return;
  const auto pos = e.getEventRelativeTo(&content_).position.toInt();
  ui::Jack* target = nullptr;
  for (auto* c = content_.getComponentAt(pos); c != nullptr; c = c->getParentComponent()) {
    if ((target = dynamic_cast<ui::Jack*>(c)) != nullptr) break;
    if (c == &content_) break;
  }
  if (target && target != dragAnchor_ && target->isOutput() != dragAnchor_->isOutput()) {
    const int out = dragAnchor_->isOutput() ? dragAnchor_->index() : target->index();
    const int in = dragAnchor_->isOutput() ? target->index() : dragAnchor_->index();
    proc_.addCable(out, in);
  }
  dragAnchor_ = nullptr;
  cableLayer_.repaint();
}

void ResonatorIntoTextureEditor::clearJack(ui::Jack& j) {
  if (j.isOutput()) proc_.removeCablesAtOutput(j.index());
  else proc_.removeCablesAtInput(j.index());
}

void ResonatorIntoTextureEditor::timerCallback() {
  const bool gen = proc_.apvts.getRawParameterValue(pid::id(pid::GEN_ON))->load() > 0.5f;
  const float zoom = static_cast<float>(static_cast<double>(proc_.apvts.state.getProperty("zoom", 1.0)));
  if (gen != genShown_ || std::abs(zoom - zoom_) > 0.001f) {
    genShown_ = gen;
    zoom_ = zoom;
    updateLayout();
  }
  if (proc_.cablesVersion() != cablesSeen_) {
    cablesSeen_ = proc_.cablesVersion();
    cablesCache_ = proc_.getCables();
  }
  auto& s = proc_.status();
  for (int o = 0; o < NUM_OUT; ++o) {
    const float v = s.outLevel[o].exchange(0.0f);
    levels_[o] = std::max(v, levels_[o] * 0.78f);
  }
  if (genShown_) {
    marbles_.tick();
    plaits_.tick();
  }
  io_.tick();
  rings_.tick();
  clouds_.tick();
  cableLayer_.repaint();
}
