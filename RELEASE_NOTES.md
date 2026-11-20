## ResonatorIntoTexture 3.0.0

Generador ambient modular (Marbles → Plaits → Rings → Clouds) en VST3 para **Linux, Windows y macOS**, construido sobre el código abierto de Mutable Instruments de Émilie Gillet.

**Descargas**
- `ResonatorIntoTexture-linux-x86_64.zip`: VST3 Linux (glibc ≥ 2.35)
- `ResonatorIntoTexture-windows-x64.zip`: VST3 Windows 64 bits
- `ResonatorIntoTexture-macos-universal.zip`: VST3 macOS universal (Intel + Apple Silicon, 10.13+). Sin firmar: ejecuta `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ResonatorIntoTexture.vst3`

Cada zip incluye el manual en PDF en inglés y castellano.

**Novedades en 3.0**
- Atenuadores en las entradas de audio (Rings IN, Clouds IN L/R) y en TO DAW; ENV retirado del panel IN/OUT
- 8 modos de Texture: los de Parasites (granular, stretch, looping delay, spectral, oliverb, resonestor) y los de mrarm84 (DRUMS y BENDER)
- Cuantización a notas para Rings y Plaits, 42 escalas, guardado y carga de presets de usuario
- Compilación y validación con pluginval en los tres sistemas en CI
