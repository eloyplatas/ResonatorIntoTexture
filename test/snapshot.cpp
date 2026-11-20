#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/Params.h"
#include <juce_gui_basics/juce_gui_basics.h>

static void snap(juce::Component& c, const juce::String& path) {
  auto img = c.createComponentSnapshot(c.getLocalBounds(), true, 1.0f);
  juce::File f(path); f.deleteFile();
  juce::FileOutputStream os(f);
  juce::PNGImageFormat().writeImageToStream(img, os);
}

int main() {
  juce::ScopedJuceInitialiser_GUI init;
  auto* proc = new ResonatorIntoTextureProcessor();
  proc->prepareToPlay(48000, 512);
  auto run = [&](int blocks) {
    juce::AudioBuffer<float> buf(2, 512); juce::MidiBuffer midi; juce::Random r(3);
    for (int b = 0; b < blocks; ++b) {
      for (int i = 0; i < 512; ++i) { float s = (b % 20 < 3) ? r.nextFloat() * 0.6f - 0.3f : 0.0f; buf.setSample(0,i,s); buf.setSample(1,i,s); }
      proc->processBlock(buf, midi);
      juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
    }
  };
  {
    std::unique_ptr<juce::AudioProcessorEditor> ed(proc->createEditor());
    ed->setVisible(true);
    proc->setCurrentProgram(21);
    run(60);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(100);
    snap(*ed, "/home/claude/RingsIntoClouds/test/ui_main.png");
    printf("main %dx%d\n", ed->getWidth(), ed->getHeight());
    proc->setCurrentProgram(7);
    run(120);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(150);
    snap(*ed, "/home/claude/RingsIntoClouds/test/ui_gen.png");
    printf("gen %dx%d\n", ed->getWidth(), ed->getHeight());

    // --- Prueba de interacción con cables (eventos de ratón sintéticos)
    auto* E = dynamic_cast<ResonatorIntoTextureEditor*>(ed.get());
    auto src = juce::Desktop::getInstance().getMainMouseSource();
    auto ev = [&](juce::Component* comp, juce::Point<float> local, bool right) {
      auto mods = right ? juce::ModifierKeys(juce::ModifierKeys::rightButtonModifier) : juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier);
      return juce::MouseEvent(src, local, mods, 1.0f, 0, 0, 0, 0, comp, comp, juce::Time::getCurrentTime(), local, juce::Time::getCurrentTime(), 1, false);
    };
    auto* jOut = E->jackFor(port::OUT_M_T2, true);
    auto* jIn = E->jackFor(port::IN_C_TRIG, false);
    const size_t before = proc->getCables().size();
    auto target = jIn->getLocalPoint(jOut, juce::Point<float>(22.0f, 27.0f));  // no usado
    // pulsar en T2, soltar sobre TRIG (coordenadas relativas al jack de origen)
    jOut->mouseDown(ev(jOut, {22, 27}, false));
    auto upPos = jOut->getLocalPoint(jIn, juce::Point<float>(22.0f, 27.0f));
    jOut->mouseUp(ev(jOut, upPos, false));
    const size_t after = proc->getCables().size();
    bool found = false; for (auto& c : proc->getCables()) if (c.out == port::OUT_M_T2 && c.in == port::IN_C_TRIG) found = true;
    printf("crear cable: %zu -> %zu, encontrado %d\n", before, after, (int)found);
    // despegar desde la entrada y soltar en vacío -> se elimina
    jIn->mouseDown(ev(jIn, {22, 27}, false));
    jIn->mouseUp(ev(jIn, {200, -300}, false));
    found = false; for (auto& c : proc->getCables()) if (c.out == port::OUT_M_T2 && c.in == port::IN_C_TRIG) found = true;
    printf("despegar y soltar fuera: encontrado %d, cables %zu\n", (int)found, proc->getCables().size());
    // ODD -> TO DAW LNESS del resonador
    {
      auto* jEnv = E->jackFor(port::OUT_R_ODD, true);
      auto* jB = E->jackFor(port::IN_IO_OUT_L, false);
      jEnv->mouseDown(ev(jEnv, {22, 27}, false));
      jEnv->mouseUp(ev(jEnv, jEnv->getLocalPoint(jB, juce::Point<float>(22.0f, 27.0f)), false));
      bool ok = false; for (auto& c : proc->getCables()) if (c.out == port::OUT_R_ODD && c.in == port::IN_IO_OUT_L) ok = true;
      printf("ODD -> TO DAW L: %d\n", (int)ok);
      // y al revés: desde la entrada hasta ENV
      auto* jD = E->jackFor(port::IN_C_DENS, false);
      jD->mouseDown(ev(jD, {22, 27}, false));
      jD->mouseUp(ev(jD, jD->getLocalPoint(jEnv, juce::Point<float>(22.0f, 27.0f)), false));
      ok = false; for (auto& c : proc->getCables()) if (c.out == port::OUT_R_ODD && c.in == port::IN_C_DENS) ok = true;
      printf("DENS -> ODD (arrastre inverso): %d\n", (int)ok);
    }
    // clic derecho en X1 quita sus cables
    auto* jx1 = E->jackFor(port::OUT_M_X1, true);
    jx1->mouseDown(ev(jx1, {22, 27}, true));
    int x1 = 0; for (auto& c : proc->getCables()) if (c.out == port::OUT_M_X1) ++x1;
    printf("clic derecho en X1: cables restantes de X1 = %d, total %zu\n", x1, proc->getCables().size());
    (void)target;
  }
  {
    // texture en modo TRIG DRUMS
    std::unique_ptr<juce::AudioProcessorEditor> ed(proc->createEditor());
    ed->setVisible(true);
    proc->setCurrentProgram(9);
    proc->apvts.getParameter(pid::id(pid::C_MODE))->setValueNotifyingHost(proc->apvts.getParameter(pid::id(pid::C_MODE))->convertTo0to1(6.0f));
    proc->addCable(port::OUT_M_T1, port::IN_C_POS);
    proc->addCable(port::OUT_M_T3, port::IN_C_SIZE);
    proc->addCable(port::OUT_M_T2, port::IN_C_TRIG);
    run(150);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(150);
    snap(*ed, "/home/claude/RingsIntoClouds/test/ui_drums.png");
  }
  delete proc;
  return 0;
}
