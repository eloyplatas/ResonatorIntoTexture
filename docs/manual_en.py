# -*- coding: utf-8 -*-
# ResonatorIntoTexture manual — English
B = []
def h1(t): B.append(("h1p", t))
def h2(t): B.append(("h2", t))
def h3(t): B.append(("h3", t))
def p(t): B.append(("p", t))
def lead(t): B.append(("lead", t))
def ul(*items): B.append(("ul", list(items)))
def tbl(rows, w): B.append(("table", rows, w))
def img(f, w, cap): B.append(("img", f, w, cap))
def small(t): B.append(("small", t))

# ======================================================================
B.append(("h1", "Dedication"))
lead("This plugin exists because of Émilie Gillet. For more than a decade, working on her own from Paris, she designed "
     "some of the most beautiful and influential electronic instruments of our time, and then did something almost "
     "nobody does: she gave her code and her schematics to the world, so that others could learn from them, repair "
     "them, change them and keep making music with them long after her company was gone.")
p("ResonatorIntoTexture does not try to replace Mutable Instruments modules, and it is not an official product. It "
  "is a tribute. It runs the original DSP exactly as she published it, without rewriting it, and simply builds it a new "
  "home inside a DAW. Every sound that comes out of it is, at heart, hers. If you enjoy this plugin, the credit belongs "
  "to Émilie; any mistakes belong to the author of this adaptation.")
p("Thanks also to <b>Matthias Puech</b>, whose <i>Parasites</i> firmware extended Clouds with imagination and respect, "
  "and to <b>mrarm84</b>, who built the <i>Trig drums</i> machine and the <i>Data bender</i> on top of Parasites. Building "
  "on what others have shared is exactly the spirit that Mutable Instruments made possible.")

# ======================================================================
h1("1. A short history of Mutable Instruments")
h2("Beginnings")
p("Mutable Instruments grew out of the curiosity of a signal-processing engineer in France. Émilie Gillet began "
  "experimenting with Arduino boards in 2009, and that work produced the first Mutable instrument, the "
  "<b>Shruthi-1</b>, a hybrid synthesizer sold as a do-it-yourself kit. Other desktop instruments followed, such as "
  "Anushri, Ambika and the MIDIpal, which Gillet phased out in 2014 to focus on Eurorack.")
p("From the start the company had an unusual trademark in the music industry: open source. Gillet placed an open-source "
  "ethos at the core of her company, and that made Mutable Instruments one of the most celebrated and "
  "consequential names in modular synthesis of the past fifteen years. Module firmware was routinely published on GitHub "
  "under an open license, so other makers could build their own hardware and run the original software.")
h2("The Eurorack years")
p("Within a few years, Mutable modules had become a familiar sight in almost any rack. They had a very particular idea of "
  "what an instrument should be: digital modules with sophisticated processing at their core, but with high-level, "
  "musical, easy-to-grasp controls, framed by clean and elegant aluminium panels. Many of them hid alternative modes "
  "and easter eggs that rewarded curiosity.")
p("The family included <b>Braids</b> and its successor <b>Plaits</b> (macro oscillators), <b>Elements</b> and <b>Rings</b> "
  "(modal synthesis and resonators), <b>Clouds</b> and its successor <b>Beads</b> (granular textures), <b>Marbles</b> "
  "(controlled randomness), <b>Tides</b>, <b>Warps</b>, <b>Stages</b>, <b>Peaks</b>, <b>Frames</b>, <b>Grids</b>, "
  "<b>Veils</b>, <b>Ripples</b>… instruments designed to work together.")
h2("The end, and what remains")
p("In April 2022, Émilie Gillet announced that no new modules would be produced, no further inventory would be made, and "
  "the company would close by the end of that year. She wrote on the company forum that no new module was in "
  "preparation, and that Beads, designed back in 2019, was the last one. In December 2022 the website "
  "confirmed that production had halted and that Mutable Instruments would permanently shut down, with support and "
  "repairs available until June 2023.")
p("Yet the story did not end there, precisely because of that decision to share everything. Thanks to open licensing, "
  "her designs have been cloned in Eurorack, translated to software such as VCV Rack, and built into hardware synths "
  "like Arturia's MicroFreak and MiniFreak. VCV Rack, for instance, offers <i>Audible "
  "Instruments</i>, an authorized port of Mutable modules. As was said at the time, her designs will keep "
  "influencing hardware and software synthesis for decades to come. This plugin is one more small "
  "example.")

# ======================================================================
h1("2. The four modules that live here")
h2("Rings — the resonator")
p("Rings appeared at the end of 2015 and brought physical-modelling synthesis to Eurorack from a very modular angle. "
  "Instead of being a complete instrument, it focuses on the key ingredient: the <i>resonator</i>, the bar, the tube or "
  "the set of strings that an external signal sets vibrating. It uses the same resonator code as Elements, with two "
  "additions: the ability to split its 60 filters into 2×30 or 4×15 for polyphony, and two comb-filter string models "
  "that sound completely different.")
p("Its appeal is immediate: give it a note and a trigger, or even nothing at all, and it answers with a living sound "
  "with its own natural decay, moving from glass and steel to wood and nylon with a single knob (BRIGHTNESS).")
h2("Clouds — the texture synthesizer")
p("Clouds, released in 2014, was one of the earliest modular hardware devices to exploit polyphonic granular synthesis. "
  "Unlike granular sample players, it focuses on the real-time granularization of incoming audio, and it can "
  "superimpose up to 16 grains at once to build thick textures. It became an instant classic: even in "
  "2020, six years after its release, it was still among the top ten modules sold on Reverb. Its "
  "successor, Beads, reinvented it in 2021.")
p("In this plugin, texture runs Matthias Puech's <b>Parasites</b> firmware, which keeps the four original modes "
  "(granular, stretch, looping delay and spectral) untouched and adds two more: Oliverb and Resonestor.")
h2("Plaits — the macro oscillator")
p("Plaits arrived in March 2018 as the spiritual successor to Braids, with hardware and software redesigned from scratch. "
  "It started with 16 synthesis models, 8 melodic and 8 percussive, each with three timbre parameters. Its philosophy "
  "was <i>one model = one synthesis technique</i>. Its final firmware update brought it to 24 models, all included here.")
h2("Marbles — the random sampler")
p("Marbles, from April 2018, is a <i>random sampler</i>: it produces random gates and voltages while offering many ways "
  "of imposing structure on them (sync to external clocks, control over repetition or novelty, quantization…). Its "
  "<b>DÉJÀ VU</b> knob controls the probability of reusing past material instead of sampling fresh random data: it is "
  "the key to <i>repeatable randomness</i>.")
h2("Parasites, Trig drums and Data bender")
p("<b>Parasites</b> is a series of alternative firmwares for Mutable modules by Matthias Puech. For Clouds it added the "
  "<b>Oliverb</b>, a creative reverb based on the original firmware's reverb and inspired by the Erbe-Verb's design, and "
  "the <b>Resonestor</b>, a string-and-chord resonator. It is distributed under the MIT licence. "
  "<b>mrarm84</b> published a firmware that keeps all six Parasites modes intact and adds <b>Trig drums</b> (an 8-voice "
  "drum machine) and <b>Data bender</b> (buffer glitching), also under the MIT licence.")

# ======================================================================
h1("3. “Rings into Clouds”")
lead("Few phrases are as recognisable in modular music. “Rings into Clouds” is at once a technique, a beginner's recipe, "
     "an ambient classic… and a running joke of the community.")
h2("The technique")
p("The idea is simple: Rings' output goes into Clouds. With Rings, Clouds and a single trigger source, one can spiral off "
  "into endless ambient atmospheres: Rings acts as a nearly all-in-one physical-modelling voice, and Clouds as a "
  "complex, shimmering granular delay/reverb.")
h2("Why it works so well")
ul("<b>Living sound from the first second.</b> A resonator needs no envelopes or filters to sound organic: every strike has "
   "an attack, a body and a natural decay, rich in evolving harmonics.",
   "<b>Ideal material for granulation.</b> Resonant, pitched sounds with long tails are perfect for Clouds to fragment, "
   "stretch, freeze and multiply without turning into noise.",
   "<b>Two modules, two dimensions.</b> Rings defines <i>pitch and timbre</i>; Clouds defines <i>space and time</i>. They "
   "complement each other instead of competing.",
   "<b>Few cables, big results.</b> With a sequence or a random source (such as Marbles) the patch evolves on its own: "
   "perfect for generative music, installations or quiet live sets.",
   "<b>The same hand behind both.</b> Both modules share a design philosophy: high-level controls, musical response "
   "across the whole range and a recognisable sonic character.")
h2("Classic, cliché and starting point")
p("The patch's very success turned it into a cliché. Perfect Circuit calls it one of the most polarising Eurorack tropes and "
  "admits it has become something of a joke, but also that there is a lot of truth to its staying power: they have seen "
  "many musicians start their modular journey with these modules as a starting point. The Dawless Jamming channel even ran "
  "an open call for meditative modular under the motto “Rings into Clouds Forever”, and as late as 2025 "
  "videos were still being made about why these two modules remain among the most popular in Eurorack.")
p("When Mutable Instruments closed, one of the farewell messages said it all: “Rings into Clouds, forever!”. "
  "ResonatorIntoTexture was born from that affection: the brand's most celebrated patch turned into a "
  "plugin, with everything you need to go beyond the cliché.")

# ======================================================================
h1("4. Installation")
tbl([["System", "Copy ResonatorIntoTexture.vst3 to", "Notes"],
     ["Linux", "~/.vst3/", "x86_64, glibc 2.35 or newer (Ubuntu 22.04+, Debian 12, Fedora, Arch)."],
     ["Windows", "C:\\Program Files\\Common Files\\VST3\\", "64-bit."],
     ["macOS", "~/Library/Audio/Plug-Ins/VST3/", "Universal (Apple Silicon and Intel), macOS 10.13 or newer. The plugin is not "
      "signed by an Apple developer: if macOS blocks it, run "
      "<font face='Mono'>xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ResonatorIntoTexture.vst3</font> in Terminal."]],
    [0.13, 0.37, 0.50])
p("Then rescan plugins in your DAW (in REAPER: <i>Options → Preferences → Plug-ins → VST → Re-scan</i>). "
  "ResonatorIntoTexture is an effect with MIDI input: put it on a track with audio, or on an empty track if you only want "
  "to use the generator.")

# ======================================================================
h1("5. Overview")
img("ui_gen.png", 1.0, "The five panels with the generator open (patch “Voces en el pozo”).")
p("ResonatorIntoTexture is a small modular system of five panels. From left to right:")
tbl([["Panel", "Based on", "Role"],
     ["random sampler", "Marbles", "Random gates (T1, T2, T3) and quantized voltages (X1, X2, X3, Y)."],
     ["macro oscillator", "Plaits 1.2", "Oscillator with 24 synthesis models."],
     ["in / out", "—", "Patches, input level, generator, MIDI, mix and output to the DAW."],
     ["resonator", "Rings", "Physical-modelling resonator: modal, sympathetic strings, string, FM…"],
     ["texture", "Clouds + Parasites + mrarm84", "Granular, stretch, delay, spectral, Oliverb, Resonestor, drum machine and bender."]],
    [0.2, 0.25, 0.55])
p("The <b>GEN</b> button on the in/out panel shows or hides the random sampler and the macro oscillator. Without them, "
  "ResonatorIntoTexture is a “Rings into Clouds” effect on the track's audio. With them, it becomes a self-playing "
  "generator.")
h2("Every module at its own sample rate")
p("The original DSP runs unmodified at its native sample rate: random sampler, macro oscillator and resonator at 48 kHz in "
  "24-sample blocks; texture at 32 kHz, like the hardware Clouds. A polyphase resampler (Kaiser-windowed sinc) bridges "
  "those domains with the DAW's sample rate. Each module keeps its character, including the slightly dark, grainy sheen "
  "of Clouds at 32 kHz.")

# ======================================================================
h1("6. Cables, voltages and attenuators")
h2("Patching")
ul("<b>Connect:</b> drag from an output (the jacks on black plates) to an input, or the other way round.",
   "<b>Move:</b> drag from an input that already has a cable to unplug it and take it elsewhere; drop it on empty space to delete it.",
   "<b>Remove:</b> right-click a jack to remove all its cables. Right-click the in/out panel background for “Remove all cables”.",
   "Several outputs patched into one input are summed. One output can feed many inputs.")
h2("Voltages")
p("Signals behave as in Eurorack: ±5 V audio (±1.0 digital), 0/5 V gates and 1 V/oct pitch. A gate counts as high above 1 V.")
h2("CV attenuverters")
p("Every control input has a <b>mini attenuverter</b> above it, from −100 % to +100 %. At +100 %, 5 V sweeps the full range "
  "of the matching knob; on pitch inputs (FM and PITCH) +100 % means 1 V/oct. When you patch a cable into an input whose "
  "attenuverter is at zero, it opens to +50 % (+100 % for FM and PITCH) so the effect is heard right away.")
h2("Audio attenuators")
p("Audio inputs have a <b>level attenuator</b> (0 to 100 %, default 100 %): the resonator's IN, texture's IN L / IN R and "
  "TO DAW L / R. They act on whatever comes in, whether patched or normalled.")
h2("Normalisations (what happens without a cable)")
tbl([["Input", "Unpatched"],
     ["resonator IN", "Depends on EXCITER: the track's audio, nothing, or the internal exciter (plus anything patched)."],
     ["resonator V/OCT and STRUM", "MIDI notes set the pitch and strum. Without MIDI: FREQUENCY and the input's onset detector."],
     ["oscillator V/OCT and TRIG", "MIDI notes play the oscillator. Without MIDI: it drones continuously."],
     ["texture IN L / R", "The resonator's ODD / EVEN outputs, cross-mixed 60/40 so no voice is stuck on one side."],
     ["TO DAW L / R", "texture's output."]],
    [0.3, 0.7])

# ======================================================================
h1("7. in / out panel")
img("p_io.png", 0.24, "The in / out panel.")
tbl([["Control", "Function"],
     ["Patch selector", "Factory and user patches. &lt; &gt; arrows: previous / next. Click the name: preset menu (see chapter 13)."],
     ["INPUT", "Track input gain (−24 to +24 dB). The LEDs show its level."],
     ["GEN", "Shows or hides the generator. With no generator cables yet, it creates the standard patch: oscillator OUT → "
      "resonator IN, T1 → TRIG, X1 → oscillator V/OCT, T3 → STRUM, X2 → resonator V/OCT, Y → TIMBRE and Y → POS."],
     ["MIDI", "Root and number of notes of the MIDI-learned scale."],
     ["MIX (+ CV)", "Balance between the dry track (latency-compensated) and the processed sound."],
     ["OUTPUT", "Output gain (−24 to +12 dB)."],
     ["TO DAW L / R", "Inputs for what you hear in the DAW, each with its level attenuator."],
     ["Right-click", "Interface zoom (60–150 %) and “Remove all cables”."]],
    [0.22, 0.78])

# ======================================================================
h1("8. random sampler (Marbles)")
img("p_marbles.png", 0.55, "The random sampler panel.")
h2("T section: random gates")
tbl([["Control", "Function"],
     ["RATE", "Internal clock rate. With SYNC or T CLK, a multiplier / divider of the external clock (centre = 1:1)."],
     ["BIAS", "Relative probability between T1 and T3."],
     ["JITTER", "Timing irregularity, from a perfect clock to “human” and chaotic."],
     ["T MODE", "Coin toss (complementary Bernoulli), Ratios, Drums, Independent, Divider, Three states, Markov."],
     ["T RANGE", "×0.25, ×1, ×4 rate."],
     ["T1 · T2 · T3 outputs", "T2 is the master clock; T1 and T3 share the pulses according to BIAS and the model."]],
    [0.22, 0.78])
h2("Déjà vu")
tbl([["Control", "Function"],
     ["DÉJÀ VU", "Probability of reusing past material. At the centre (dead zone) the loop is locked and repeats exactly."],
     ["LENGTH", "Loop length (1 to 16 steps)."],
     ["T / X", "Enable déjà vu for the gates and/or the voltages."]],
    [0.22, 0.78])
h2("X section: random voltages")
tbl([["Control", "Function"],
     ["SPREAD", "Width of the distribution, from almost constant to very wide."],
     ["BIAS", "Moves the distribution up or down."],
     ["STEPS", "Left: smoothed voltages; centre: raw; right: quantized to the scale, increasingly restricted to its most important degrees."],
     ["X RANGE", "+2 V, +5 V or ±5 V."],
     ["SCALE", "Click: grouped list of 42 scales, plus “MIDI” (scale learned from the keyboard)."],
     ["ROOT", "Root note for the factory scales."],
     ["X1 · X2 · X3 · Y outputs", "X1–X3 are quantized and transposed to the root; Y is a slow modulation (0–5 V)."]],
    [0.22, 0.78])
h2("Scales")
p("<b>Marbles:</b> Major, Minor, Pentatonic, Pelog, Raag Bhairav, Raag Shri (original weights). <b>Modes:</b> Ionian, "
  "Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian, harmonic minor, melodic minor, Lydian dominant, Phrygian "
  "dominant. <b>Pentatonics:</b> major, minor, blues, Hirajoshi, In sen, Iwato, Kumoi, Yo. <b>Symmetric:</b> chromatic, "
  "whole tone, octatonic H-W and W-H, augmented, fifths, octaves. <b>World:</b> double harmonic, Hungarian minor, Raag "
  "Todi, Prometheus, enigmatic. <b>Microtonal:</b> Slendro, Maqam Rast, Maqam Bayati, just major, harmonic series.")
p("<b>MIDI scale:</b> choose “MIDI” in SCALE and play or hold a chord. The held notes become the scale and the lowest one "
  "is the root. It is remembered when you let go and saved with the project.")
h2("Sync and external clock")
p("<b>SYNC</b> locks the clock to the DAW transport; <b>DIV</b> chooses the division (1/1, 1/2, 1/4, 1/8, 1/16 and "
  "triplets). When the transport stops, the generator stops too. The <b>T CLK</b> input accepts a patched clock and takes "
  "priority over SYNC.")

# ======================================================================
h1("9. macro oscillator (Plaits 1.2)")
img("p_plaits.png", 0.4, "The macro oscillator panel.")
tbl([["Control", "Function"],
     ["MODEL ◀ ▶", "Selects one of 24 models. LED colour shows the bank: orange (Plaits 1.2's new models), green and red."],
     ["FREQUENCY", "Tuning (±24 semitones around C3)."],
     ["HARMONICS · TIMBRE · MORPH", "The three timbre parameters; their meaning depends on the model."],
     ["DECAY · LPG COLOUR", "Decay and character of the low-pass gate when the oscillator is triggered (TRIG)."],
     ["LEVEL", "Output level."],
     ["NOTES", "Quantization of the final note: free, semitones or the random sampler's scale."],
     ["Inputs", "FM, HARMO, TIMBRE, MORPH, DECAY, LPG, LEVEL, MODEL (with attenuverter), V/OCT and TRIG."],
     ["Outputs", "OUT and AUX (a variant or by-product of the main signal)."]],
    [0.27, 0.73])
tbl([["Orange bank", "Green bank", "Red bank"],
     ["VA-VCF (analog with filter)", "Virtual analog", "Swarm (oscillator swarm)"],
     ["Phase distortion", "Waveshaping", "Filtered noise"],
     ["6-op FM I", "2-op FM", "Particles (particle noise)"],
     ["6-op FM II", "Formant grain", "Inharmonic string"],
     ["6-op FM III", "Additive (harmonics)", "Modal resonator"],
     ["Wave terrain", "Wavetable", "Bass drum"],
     ["String machine", "Chords", "Snare drum"],
     ["Chiptune", "Speech", "Hi-hat"]],
    [0.333, 0.333, 0.334])

# ======================================================================
h1("10. resonator (Rings)")
img("p_rings.png", 0.5, "The resonator panel.")
tbl([["Control", "Function"],
     ["FREQUENCY", "Resonator root (60 semitones). With V/OCT or MIDI it is rounded to semitones, as on the module."],
     ["STRUCTURE", "Modal: inharmonicity and geometry of the object. Sympathetic strings: intervals between strings. String: dispersion."],
     ["BRIGHTNESS", "Harmonic content, from wood and nylon to glass and steel."],
     ["DAMPING", "Length of the resonance."],
     ["POSITION", "Excitation point (like plucking a string near the bridge or the middle)."],
     ["RESONATOR", "Modal, Sympathetic strings, Inharmonic string and the bonus modes: FM voice, Sympathetic quantized, String + reverb (red LEDs)."],
     ["POLYPHONY", "1, 2 or 4 voices. Each STRUM moves to the next voice and the tails overlap."],
     ["EXCITER", "Track + cables: the track's audio plus anything patched into IN. Cables only: just what is patched into IN. Internal + cables: the internal exciter, which strikes on every STRUM."],
     ["NOTES", "Quantization: free, semitones or the random sampler's scale."],
     ["ON", "When off, the resonator passes its input through unprocessed."],
     ["Inputs", "FM, STRUC, BRIGHT, DAMP, POS (with attenuverter), STRUM, V/OCT and IN (with level attenuator)."],
     ["Outputs", "ODD and EVEN: with polyphony 1, two sets of harmonics; with 2 or 4 voices, alternate voices."]],
    [0.2, 0.8])
p("<b>Onset detector.</b> With nothing patched into STRUM and no MIDI, the resonator strums itself whenever it detects an "
  "attack in the input signal, just like the original module when only IN is patched.")

# ======================================================================
h1("11. texture (Clouds + Parasites + mrarm84)")
img("p_clouds.png", 0.55, "The texture panel in granular mode.")
h2("Common controls")
tbl([["Control", "Function"],
     ["MODE", "Click: list of the 8 modes. Knob names change with the mode."],
     ["QUALITY", "16-bit stereo, 16-bit mono, 8-bit stereo, 8-bit mono (longer, lo-fi buffer)."],
     ["FREEZE", "Freezes the buffer (in Trig drums it enables DUB)."],
     ["PERF FX", "Performance effects in Trig drums and Data bender."],
     ["Options A / B", "Mode-dependent: GRAINS, VOICE, KIT/AUDIO IN, ENGINE, CORRUPT."],
     ["ON", "When off, texture passes its input through."],
     ["Inputs", "9 CV inputs with attenuverters, FREEZE, TRIG, IN L / IN R (with level attenuators)."],
     ["Outputs", "OUT L and OUT R."]],
    [0.2, 0.8])
h2("The 8 modes and their knobs")
tbl([["Mode", "POS", "SIZE", "PITCH", "DENS", "TEX", "BLEND", "SPREAD", "FDBK", "VERB"],
     ["Granular", "position", "size", "pitch", "density", "texture", "dry/wet", "stereo", "feedback", "reverb"],
     ["Stretch", "position", "window", "pitch", "diffusion", "filter", "dry/wet", "stereo", "feedback", "reverb"],
     ["Looping", "delay", "size", "pitch", "diffusion", "filter", "dry/wet", "stereo", "feedback", "reverb"],
     ["Spectral", "position", "size", "pitch", "quantize", "texture", "dry/wet", "stereo", "feedback", "reverb"],
     ["Oliverb", "pre-delay", "room", "shift", "decay", "LP/HP", "dry/wet", "diffusion", "mod rate", "mod amt"],
     ["Resonestor", "burst", "chord", "pitch", "feedback", "filter", "distortion", "stereo", "harmonic", "voice spread"],
     ["Trig drums", "char A", "contour", "tune", "char B", "filter", "send", "pan", "echo fb", "verb tail"],
     ["Data bender", "time", "repeats", "bend", "break", "corrupt", "mix", "width", "regen", "rev. chance"]],
    [0.11] + [0.099] * 9)
h3("Granular")
p("The classic Clouds mode. DENSITY at 12 o'clock stops grain generation: to the left grains appear regularly, to the "
  "right at random. TRIG fires a grain. The GRAINS option (from Parasites) adds reversed grains.")
h3("Stretch")
p("A WSOLA time-stretcher / pitch-shifter. With PITCH at 0 and FREEZE off the sound barely changes: that is expected. Its "
  "magic appears when you change PITCH without changing duration, or freeze the buffer and scan it with POSITION.")
h3("Looping delay and Spectral")
p("A pitch-shifting looping delay, and a spectral (phase vocoder) processor that quantizes and blurs the spectrum.")
h3("Oliverb")
p("Parasites' creative reverb: size, decay, LP/HP filter, diffusion and a modulation that, at slow rates and high amounts, "
  "produces sustained, pitched sounds. PITCH shifts the tail's pitch; FREEZE gives an infinite reverb.")
h3("Resonestor")
p("Parasites' string-and-chord resonator: TRIG excites a burst, SIZE picks the chord and PITCH the note.")
h3("Trig drums")
p("An 8-voice drum machine. texture's CV jacks become triggers and their labels change:")
tbl([["Jack", "POS", "SIZE", "PITCH", "TRIG", "DENS", "TEX", "BLEND", "FREEZE"],
     ["Voice", "BD kick", "SD snare", "OH open hat", "CH closed hat (also the echo clock)", "CP clap", "MT multi (tom, conga, rim, cowbell)", "SA chord stab", "SB resonator"]],
    [0.1] + [0.1125] * 8)
ul("<b>VOICE (KNOBS):</b> which voice the knobs edit. Each voice keeps its values; a knob doesn't jump until you move it.",
   "<b>KIT / AUDIO IN:</b> Analog (round) or Punch (clicky) kit. The input can pass through (<i>Thru</i>), become the "
   "voices' noise (<i>Skin</i>) or be heard only through the hits (<i>Chop</i>).",
   "<b>FREEZE = DUB:</b> longer decays, everything into the echo and reverb.",
   "To play it from the generator: T1 → POS (kick), T3 → SIZE (snare), T2 → TRIG (hats).")
h3("Data bender")
p("Buffer glitching: the input is recorded into a one-second buffer and played back as repeated, reversed, sped-up, broken "
  "and corrupted slices. ENGINE selects Traverse, Silence or Micro (each also reversed); CORRUPT selects Decimate, "
  "Dropout, Destroy, DJ filter or Vinyl. TRIG is the clock, FREEZE freezes and the PITCH input works as V/OCT (Micro speed).")
h3("PERF FX")
p("In Trig drums and Data bender: Stutter, Tape stop, Filter sweep, Bitcrush, Wash, Reverse, Gate and Echo throw (Octave "
  "down in Data bender), synced to the beat.")
p("<b>Note:</b> the Trig drums and Data bender engines were written for a hardware module and use global state, so only "
  "one plugin instance at a time can use those two modes; if another instance has them, a red notice appears. The other "
  "six modes have no such limit.")

# ======================================================================
h1("12. MIDI")
ul("<b>Without the generator:</b> each note tunes and strums the resonator; FREQUENCY transposes (centre = exact pitch).",
   "<b>With the generator:</b> if the oscillator's V/OCT and TRIG are unpatched, MIDI notes play it.",
   "<b>MIDI scale:</b> with SCALE set to “MIDI”, held notes define the random sampler's scale and the NOTES quantization.")

# ======================================================================
h1("13. Patches and user presets")
h2("Factory patches")
tbl([["Generative (self-playing)", "Effects (for the track's audio)"],
     ["01 Lluvia de cristal · 02 Órgano de niebla · 03 Kalimba aleatoria · 04 Arpa eólica · 05 Pelog nocturno · "
      "06 Coro de partículas · 07 Voces en el pozo · 08 Enjambre · 09 Tambores de lata · 10 Cadena de Markov · "
      "11 Glaciar · 12 Campanas tibetanas · 13 Máquina de cuerdas · 14 Chiptune en la nieve · 15 Seis operadores · "
      "16 Terreno ondulado · 17 Ruido del mar",
      "18 Campana sobre la voz · 19 Cuerdas simpáticas · 20 Sitar eléctrico · 21 Espectro que respira · "
      "22 Eco de ruiseñor · 23 Gamelan de entrada · 24 Bajo lejano · 25 Respira"]],
    [0.6, 0.4])
p("All are levelled around −22 dB RMS and also appear as VST3 programs in the DAW.")
h2("User presets")
p("Click the patch name: <b>Fábrica</b> (factory), <b>Usuario</b> (yours, shown in yellow), <b>Guardar</b> (save), "
  "<b>Guardar como…</b> (save as), <b>Cargar archivo…</b> (load file), <b>Exportar a archivo…</b> (export), <b>Borrar</b> "
  "(delete) and <b>Abrir carpeta de presets</b> (open folder). Presets are <font face='Mono'>.ritpreset</font> (XML) files "
  "holding parameters, cables, the MIDI scale and the drum machine's per-voice settings.")
tbl([["System", "Preset folder"],
     ["Linux", "~/.config/ResonatorIntoTexture/Presets/"],
     ["Windows", "%APPDATA%\\ResonatorIntoTexture\\Presets\\"],
     ["macOS", "~/Library/ResonatorIntoTexture/Presets/"]],
    [0.25, 0.75])

# ======================================================================
h1("14. Recipes")
h2("The classic Rings into Clouds")
ul("Load <b>Init</b> and press <b>GEN</b>: the standard patch appears.",
   "Resonator: EXCITER on <i>Internal + cables</i>, RESONATOR on <i>Sympathetic strings</i>, POLYPHONY 4, high DAMPING.",
   "Random sampler: a pentatonic SCALE, STEPS to the right, slow RATE, DÉJÀ VU near the centre.",
   "Texture: Granular mode, DENSITY at 2 o'clock, large SIZE, REVERB and FEEDBACK halfway.",
   "Set MIX and OUTPUT, press play and let it evolve.")
h2("An effect with a life of its own")
ul("Put the plugin on a guitar or vocal track, without GEN.",
   "RESONATOR on Modal, STRUCTURE halfway, NOTES on <i>Scale</i> with the song's key as ROOT.",
   "Texture on Oliverb, high DECAY, SHIFT +12 for an octave shimmer.")
h2("Granular rhythm")
ul("GEN on, T MODE on <i>Drums</i>, texture on <i>Trig drums</i>.",
   "T1 → POS (kick), T3 → SIZE (snare), T2 → TRIG (hats); KIT on Punch, PERF FX on Stutter.")

# ======================================================================
h1("15. Technical notes")
ul("Latency reported to the host: about 6 ms (DAWs compensate for it).",
   "Typical CPU use: 1.5 % to 5.5 % of one core, depending on the patch.",
   "Fix compared with early versions: the resonator was being reset on every audio block and never rang; since 2.0 it "
   "rings with its natural decay.",
   "Panel names are functional (resonator, texture…) because the README of the Mutable Instruments repository asks that "
   "derivative works not use its trademark or the original module names.")

# ======================================================================
h1("16. Credits and licences")
tbl([["Component", "Author", "Licence"],
     ["Rings, Plaits, Marbles, Clouds DSP and stmlib", "Émilie Gillet / Mutable Instruments", "MIT"],
     ["Parasites (Oliverb, Resonestor…)", "Matthias Puech", "MIT"],
     ["Trig drums, Data bender, perf FX", "mrarm84", "MIT"],
     ["JUCE", "Raw Material Software", "AGPLv3 (JUCE 8 open-source licence)"],
     ["ResonatorIntoTexture (plugin, interface, cables, presets)", "Eloy Platas", "AGPLv3"]],
    [0.42, 0.33, 0.25])
small("Mutable Instruments is a registered trademark. This project is not affiliated with or endorsed by Mutable Instruments.")
h2("Sources")
small("Perfect Circuit, “Mutable Instruments Retrospective” and “Rings into Clouds” · Synthtopia, “Mutable Instruments R.I.P.” "
      "(Dec. 2022) and “Mutable Instruments Rings Into Clouds, A Classic Modular Combo” (2025) · MusicRadar on Mutable "
      "Instruments' closure (2022) · gearnews, “Mutable Instruments: No new modules” · Perfect Circuit, “Mutable Instruments "
      "Beads” · Reverb, “5 Fun Things to Do With Beads” · mutable-instruments.net/modules/rings · Resident Advisor (Plaits and "
      "Marbles, 2018) · VCV Rack, Audible Instruments · Synthtopia, “Parasite brings new features to Clouds” · "
      "mqtthiqs.github.io/parasites · CDM, “Eurorack ambient radio” (2019) · MetaFilter, “Au revoir Mutable Instruments” (2022) · "
      "github.com/pichenettes/eurorack · github.com/mqtthiqs/parasites · github.com/mrarm84/customFWCloudsWithDrumMachineAndBender")

CONTENT = {
    "title": "ResonatorIntoTexture — Manual",
    "footer": "ResonatorIntoTexture 3.0 · User manual (English)",
    "toc": "Contents",
    "cover_sub": "A modular tribute to Mutable Instruments",
    "cover_line": "Marbles · Plaits · Rings · Clouds (Parasites) in a patchable VST3 plugin",
    "cover_ver": "Version 3.0 · User manual · Eloy Platas",
    "blocks": B,
}
