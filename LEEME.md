# ResonatorIntoTexture 3.0 — VST3 para Linux, Windows y macOS

Sistema modular de cinco paneles con cables: **random sampler → macro oscillator → in/out → resonator → texture**.

DSP original:
- **Émilie Gillet** (`pichenettes/eurorack`, MIT): Marbles, Plaits 1.2 y Rings.
- **texture:** el firmware **Parasites** de Clouds (Matthias Puech, `mqtthiqs/parasites`, MIT) más **TRIG DRUMS** y **DATA BENDER** de mrarm84 (`customFWCloudsWithDrumMachineAndBender`, MIT).

Cada módulo corre a su frecuencia de muestreo nativa.

## Instalación
| Sistema | Copia `ResonatorIntoTexture.vst3` en |
|---|---|
| Linux (x86_64, glibc ≥ 2.35) | `~/.vst3/` |
| Windows (x64) | `C:\Program Files\Common Files\VST3\` |
| macOS (Apple Silicon + Intel, ≥ 10.13) | `~/Library/Audio/Plug-Ins/VST3/` |

En macOS el plugin no está notarizado; si se bloquea:
`xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ResonatorIntoTexture.vst3`

Después, vuelve a escanear los plugins (en REAPER: *Options → Preferences → Plug-ins → VST → Re-scan*).
El manual completo está en `docs/` (PDF, en castellano e inglés).

## Novedades de la 3.0
- **texture con 8 modos:** Granular, Stretch, Looping delay, Spectral, **Oliverb**, **Resonestor**, **Trig drums** y **Data bender**.
- **Efectos de actuación** (PERF FX) en Trig drums y Data bender: Stutter, Tape stop, Filter sweep, Bitcrush, Wash, Reverse, Gate y Echo throw (u Octave down en Data bender).
- **Cuantización de notas (NOTES)** en el resonador y en el oscilador: libre, semitonos, o la escala del random sampler.
- **Presets de usuario:** guardar, guardar como, cargar, exportar y borrar.
- **in/out simplificado:** desaparecen IN L/R y ENV; las entradas hacia REAPER se llaman **TO DAW**.
- **Atenuadores de nivel** en las entradas de audio: IN del resonador, IN L/R de texture y TO DAW L/R.
- **Versiones para Windows y macOS** (compiladas con GitHub Actions).
- **SOURCE desaparece del oscilador:** la elección del excitador pasa al resonador (EXCITER).

## El excitador del resonador (EXCITER)
Antes, SOURCE estaba en el oscilador pero decidía qué entraba al resonador. Con cables eso era redundante, y algunas opciones no se podían hacer de otra forma. Ahora el resonador tiene su propio selector:
- **Track + cables:** el audio de la pista de REAPER, más lo que conectes a IN.
- **Cables only:** solo lo que conectes a IN (por ejemplo, macro oscillator OUT → IN).
- **Internal + cables:** el excitador interno del resonador (golpe o pulso en cada STRUM), más los cables.

El audio de la pista solo entra por aquí, así que ya no hace falta un jack IN en in/out.

## texture: los 8 modos
Pulsa **MODE** para ver la lista. Los rótulos de los knobs cambian según el modo.

| Modo | POS | SIZE | PITCH | DENS | TEX | BLEND | SPREAD | FDBK | VERB |
|---|---|---|---|---|---|---|---|---|---|
| Granular | position | size | pitch | density | texture | dry/wet | spread | feedback | reverb |
| Stretch | position | window | pitch | diffusion | filter | dry/wet | spread | feedback | reverb |
| Looping | delay | size | pitch | diffusion | filter | dry/wet | spread | feedback | reverb |
| Spectral | position | size | pitch | quantize | texture | dry/wet | spread | feedback | reverb |
| Oliverb | pre-delay | room size | shift | decay | filter (LP/HP) | dry/wet | diffusion | mod rate | mod amount |
| Resonestor | burst | chord | pitch | feedback | filter | distortion | stereo | harmonicity | voice spread |
| Trig drums | char A | contour | tune | char B | filter | send | pan width | echo fb | reverb tail |
| Data bender | time | repeats | bend | break | corrupt | mix | width | regen | reverse chance |

- **Granular:** GRAINS = Forward / Reverse (granos invertidos de Parasites).
- **Stretch:** es un time-stretcher/pitch-shifter WSOLA. Con PITCH a 0 y sin FREEZE deja el audio casi igual: es lo esperado. Lo interesante es cambiar PITCH sin cambiar la duración, o congelar (FREEZE) y recorrer el buffer con POSITION. Medido: PITCH +12 sobre un La de 440 Hz da 879 Hz.
- **Oliverb / Resonestor:** los modos de Parasites. TRIG dispara la ráfaga en Resonestor.
- **Trig drums:** caja de ritmos de 8 voces. Los jacks de CV de texture pasan a ser disparos (sus rótulos cambian):

  | Jack | POS | SIZE | PITCH | TRIG | DENS | TEX | BLEND | FREEZE |
  |---|---|---|---|---|---|---|---|---|
  | Voz | BD bombo | SD caja | OH charles abierto | CH charles cerrado (también reloj del eco) | CP palmas | MT multi | SA acorde | SB resonador |

  - VOICE (KNOBS) elige qué voz editan los knobs; cada voz guarda sus valores, que no saltan hasta que mueves el knob.
  - KIT / AUDIO IN: kit Analog o Punch. El audio de entrada puede pasar (*Thru*), convertirse en el ruido de las voces (*Skin*) o oírse solo a través de los golpes (*Chop*).
  - FREEZE activa **DUB**: colas más largas y todo al eco y la reverb.
  - Para hacerla sonar con el generador: T1 → POS (bombo), T3 → SIZE (caja), T2 → TRIG (charles).
- **Data bender:** glitch de buffer.
  - ENGINE: Traverse, Silence o Micro, cada uno también en reverso.
  - CORRUPT: Decimate, Dropout, Destroy, DJ filter o Vinyl.
  - TRIG es el reloj, FREEZE congela y la entrada PITCH funciona como V/OCT (velocidad en Micro).

Los motores de Trig drums y Data bender se escribieron para un módulo físico y usan estado global. Por eso **solo una instancia del plugin a la vez** puede usar esos dos modos; si otra los tiene ocupados, aparece un aviso rojo "en uso por otra instancia". Los otros 6 modos no tienen esta limitación.

## Cuantización de notas (NOTES)
En el resonador y en el oscilador:
- **Free:** sin cuantizar.
- **Semitones:** redondea al semitono.
- **Scale:** ajusta a la escala elegida en SCALE del random sampler (con su ROOT, o la escala aprendida por MIDI).

Se aplica a la nota final: knob FREQUENCY + V/OCT + FM.

## Cables
- **Conectar:** arrastra de una salida (placa negra) a una entrada, o al revés.
- **Mover:** arrastra desde una entrada con cable para despegarlo; si lo sueltas en vacío, se elimina.
- **Quitar:** clic derecho sobre un jack.
- Cada entrada de CV tiene un atenuversor mini. Al conectar un cable, si estaba a cero se abre a +50 %, o a +100 % en FM y PITCH.
- Las entradas de audio (IN del resonador, IN L/R de texture, TO DAW L/R) tienen un atenuador de nivel (0–100 %).

### in / out
- **TO DAW L / R** (entradas, con atenuador de nivel): lo que suena en REAPER. Sin cable, la salida de texture.
- **MIX + CV:** mezcla de la señal seca de la pista con la procesada.

### Sin cable (normalizaciones)
- **texture IN L/R:** el resonador, con sus voces cruzadas 60/40.
- **resonator V/OCT y STRUM:** el MIDI y, sin MIDI, FREQUENCY y el detector de ataques.
- **oscillator TRIG:** el MIDI y, sin MIDI, suena continuo.

## Presets
En el selector del panel in/out. Las flechas < > recorren los de fábrica y los tuyos; con clic se abre el menú:
- **Fábrica:** los 25 patches más Init.
- **Usuario:** los tuyos (se muestran en amarillo).
- **Guardar** (sobrescribe el preset de usuario actual), **Guardar como…**, **Cargar archivo…**, **Exportar a archivo…**, **Borrar** y **Abrir carpeta de presets**.

Se guardan en `~/.config/ResonatorIntoTexture/Presets/` (Linux), `%APPDATA%\ResonatorIntoTexture\Presets\` (Windows) o `~/Library/ResonatorIntoTexture/Presets/` (macOS), como archivos `.ritpreset` (XML). Incluyen parámetros, cables, escala MIDI aprendida y los ajustes por voz de la caja de ritmos.

## Escalas y sincronía
- **SCALE:** 42 escalas más la aprendida por MIDI.
- **SYNC / DIV:** sincronía con REAPER. Con SYNC, RATE multiplica o divide el reloj.
- **T CLK:** reloj por cable.

## Patches de fábrica
**Generativos:** 01 Lluvia de cristal · 02 Órgano de niebla · 03 Kalimba aleatoria · 04 Arpa eólica · 05 Pelog nocturno · 06 Coro de partículas · 07 Voces en el pozo · 08 Enjambre · 09 Tambores de lata · 10 Cadena de Markov · 11 Glaciar · 12 Campanas tibetanas · 13 Máquina de cuerdas · 14 Chiptune en la nieve · 15 Seis operadores · 16 Terreno ondulado · 17 Ruido del mar

**Efectos:** 18 Campana sobre la voz · 19 Cuerdas simpáticas · 20 Sitar eléctrico · 21 Espectro que respira · 22 Eco de ruiseñor · 23 Gamelan de entrada · 24 Bajo lejano · 25 Respira

## Notas
- Latencia reportada al host: unos 6 ms. REAPER la compensa.
- Clic derecho en el panel in/out: zoom y "Quitar todos los cables".
- Nombre derivado: el README de `pichenettes/eurorack` pide no usar la marca Mutable Instruments ni los nombres originales de los módulos en obras derivadas. Por eso los paneles usan nombres funcionales.

## Compilar desde el código
```
git clone https://github.com/eloyplatas/resonatorintotexture.git
cd resonatorintotexture
git clone https://github.com/juce-framework/JUCE.git external/JUCE
git clone --recurse-submodules https://github.com/pichenettes/eurorack.git external/eurorack
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ResonatorIntoTexture_VST3
```
En Windows: Visual Studio 2022 con clang-cl (`-G "Visual Studio 17 2022" -A x64 -T ClangCL`). En macOS, binario universal con `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`.
El DSP de texture (Parasites + mrarm84) va incluido en `third_party/clouds_fw`, con su licencia MIT.

## Licencia
ResonatorIntoTexture se publica bajo la **GNU AGPLv3** (`LICENSE.md`), como exige la licencia libre de JUCE 8. El DSP de Mutable Instruments, Parasites y mrarm84 conserva su licencia MIT.
