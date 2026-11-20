# -*- coding: utf-8 -*-
# Manual de ResonatorIntoTexture — castellano
B = []
def h1(t): B.append(("h1p", t))
def h2(t): B.append(("h2", t))
def h3(t): B.append(("h3", t))
def p(t): B.append(("p", t))
def lead(t): B.append(("lead", t))
def quote(t): B.append(("quote", t))
def ul(*items): B.append(("ul", list(items)))
def tbl(rows, w): B.append(("table", rows, w))
def img(f, w, cap): B.append(("img", f, w, cap))
def code(t): B.append(("code", t))
def small(t): B.append(("small", t))

# ======================================================================
B.append(("h1", "Dedicatoria"))
lead("Este plugin existe gracias a Émilie Gillet. Durante más de una década diseñó, en solitario y desde París, "
     "algunos de los instrumentos electrónicos más bellos e influyentes de nuestro tiempo, y luego hizo algo que casi "
     "nadie hace: regaló su código y sus esquemas al mundo para que otros pudieran aprender de ellos, repararlos, "
     "modificarlos y seguir haciendo música con ellos cuando su empresa ya no existiera.")
p("ResonatorIntoTexture no pretende sustituir a los módulos de Mutable Instruments, ni presentarse como un producto "
  "oficial: es un homenaje. Usa el DSP original, sin reescribirlo, tal y como ella lo publicó, y se limita a "
  "construirle una casa nueva dentro de un DAW. Cada sonido que salga de él es, en el fondo, suyo. Si este plugin te "
  "gusta, el mérito es de Émilie; los errores, del autor de esta adaptación.")
p("Gracias también a <b>Matthias Puech</b>, cuyo firmware <i>Parasites</i> amplió Clouds con imaginación y respeto, "
  "y a <b>mrarm84</b>, que construyó sobre Parasites la caja de ritmos <i>Trig drums</i> y el <i>Data bender</i>. Ese "
  "gesto de construir sobre lo que otros compartieron es exactamente el espíritu que Mutable Instruments hizo posible.")

# ======================================================================
h1("1. Una pequeña historia de Mutable Instruments")
h2("Los comienzos")
p("Mutable Instruments nació en Francia de la curiosidad de una ingeniera de procesado de señal. Émilie Gillet empezó "
  "a experimentar con placas Arduino en 2009, y de ese trabajo surgió el primer instrumento de la marca, el "
  "<b>Shruthi-1</b>, un sintetizador híbrido que se vendía como kit para montar uno mismo. Le siguieron otros "
  "instrumentos de sobremesa, como Anushri, Ambika o el MIDIpal, que Gillet fue retirando en 2014 para concentrarse "
  "en el formato Eurorack.")
p("Desde el principio, la marca tuvo una seña de identidad poco habitual en la industria: el código abierto. Gillet "
  "situó el código abierto en el centro de su empresa, y eso convirtió a Mutable Instruments en uno de los nombres más "
  "celebrados e influyentes de la síntesis modular de los últimos quince años. El firmware de cada módulo se publicaba en GitHub "
  "con licencia libre, lo que permitió a otros fabricantes construir sus propias versiones de hardware y ejecutar el "
  "software original.")
h2("La era Eurorack")
p("En pocos años, los módulos de Mutable se convirtieron en presencia habitual en casi cualquier rack. Tenían una forma "
  "muy particular de entender el instrumento: módulos digitales con un corazón de procesado sofisticado, pero con "
  "controles de alto nivel, musicales y fáciles de entender, envueltos en paneles de aluminio limpios y elegantes. "
  "Muchos escondían modos alternativos y huevos de Pascua que recompensaban la curiosidad.")
p("Entre ellos estaban <b>Braids</b> y su sucesor <b>Plaits</b> (osciladores macro), <b>Elements</b> y <b>Rings</b> "
  "(síntesis modal y resonadores), <b>Clouds</b> y su sucesor <b>Beads</b> (texturas granulares), <b>Marbles</b> "
  "(aleatoriedad controlada), <b>Tides</b>, <b>Warps</b>, <b>Stages</b>, <b>Peaks</b>, <b>Frames</b>, <b>Grids</b>, "
  "<b>Veils</b>, <b>Ripples</b>… Una familia de instrumentos pensados para trabajar juntos.")
h2("El final, y lo que queda")
p("En abril de 2022, Émilie Gillet anunció que no se producirían más módulos nuevos, que no se fabricaría más "
  "inventario y que la empresa cerraría a final de año. Escribió en el foro de la marca que no había ningún módulo "
  "nuevo en preparación y que Beads había sido el último que diseñó, en 2019. En diciembre de 2022 la "
  "web confirmó que la producción se había detenido y que Mutable Instruments cerraba definitivamente, manteniendo "
  "soporte y reparaciones hasta junio de 2023.")
p("Pero la historia no terminó ahí, precisamente por aquella decisión de compartirlo todo. Gracias a las licencias "
  "libres, sus diseños se han clonado en Eurorack, se han trasladado a software como VCV Rack y se han incorporado a "
  "sintetizadores como el MicroFreak y el MiniFreak de Arturia. VCV Rack, por ejemplo, ofrece "
  "<i>Audible Instruments</i>, un port autorizado de los módulos de Mutable. Como se dijo entonces, sus diseños "
  "seguirán influyendo en la síntesis por hardware y por software durante décadas. Este plugin es un pequeño "
  "ejemplo más de ello.")

# ======================================================================
h1("2. Los cuatro módulos que viven aquí")
h2("Rings — el resonador")
p("Rings apareció a finales de 2015 y llevó la síntesis por modelado físico al Eurorack con un enfoque muy modular. "
  "En lugar de ser un instrumento completo, se concentra en el ingrediente esencial: el <i>resonador</i>, la barra, el "
  "tubo o el conjunto de cuerdas que se hace vibrar con una señal externa. Usa el mismo código de resonador que Elements, "
  "con dos añadidos: la posibilidad de repartir sus 60 filtros en 2×30 o 4×15 para tocar polifónicamente, y dos "
  "modelos de cuerda basados en filtros peine que suenan completamente distintos.")
p("Su atractivo es inmediato: le das una nota y un disparo, o incluso nada, y responde con un sonido vivo, con su "
  "propia caída natural, que puede ir del cristal y el metal a la madera y el nailon con un solo knob (BRIGHTNESS).")
h2("Clouds — el sintetizador de texturas")
p("Clouds, de 2014, fue uno de los primeros dispositivos de hardware en formato modular en explotar la síntesis "
  "granular polifónica. A diferencia de los reproductores granulares de muestras, se centra en granular en tiempo real "
  "el audio que entra, y puede superponer hasta 16 granos a la vez para crear texturas densas. Se convirtió en "
  "un clásico inmediato: incluso en 2020, seis años después de su lanzamiento, seguía entre los diez módulos más "
  "vendidos en Reverb. Su sucesor, Beads, lo reinventó en 2021.")
p("En este plugin, texture usa el firmware <b>Parasites</b> de Matthias Puech, que mantiene intactos los cuatro modos "
  "originales (granular, stretch, looping delay y spectral) y añade dos más: Oliverb y Resonestor.")
h2("Plaits — el oscilador macro")
p("Plaits llegó en marzo de 2018 como sucesor espiritual de Braids, con hardware y software rediseñados desde cero. "
  "Partía de 16 modelos de síntesis, 8 melódicos y 8 percusivos, cada uno con tres parámetros de timbre. Su "
  "filosofía era <i>un modelo = una técnica de síntesis</i>. Con su última actualización de firmware llegó a 24 modelos, "
  "que son los que incluye este plugin.")
h2("Marbles — el muestreador aleatorio")
p("Marbles, de abril de 2018, se describe como un <i>random sampler</i>: genera puertas y voltajes aleatorios, pero "
  "ofrece muchas formas de imponer estructura sobre ese azar (sincronía con relojes externos, control de la repetición o "
  "de la novedad, cuantización…). Su knob <b>DÉJÀ VU</b> controla la probabilidad de reutilizar material pasado en lugar "
  "de muestrear azar nuevo: es la llave de la <i>aleatoriedad repetible</i>.")
h2("Parasites, Trig drums y Data bender")
p("<b>Parasites</b> es una serie de firmwares alternativos para módulos de Mutable, creada por Matthias Puech. Para "
  "Clouds añadió el <b>Oliverb</b>, una reverb creativa basada en la reverb del firmware original e inspirada en el "
  "diseño de la Erbe-Verb, y el <b>Resonestor</b>, un resonador de cuerdas y acordes. Se distribuye con "
  "licencia MIT. <b>mrarm84</b> publicó un firmware que conserva intactos los seis modos de Parasites y añade "
  "<b>Trig drums</b> (caja de ritmos de 8 voces) y <b>Data bender</b> (glitch de buffer), también con licencia MIT.")

# ======================================================================
h1("3. «Rings into Clouds»")
lead("Pocas frases son tan reconocibles en la música modular. «Rings into Clouds» es, a la vez, una técnica, una "
     "receta para principiantes, un clásico del ambient… y una broma recurrente de la comunidad.")
h2("La técnica")
p("La idea es sencilla: la salida de Rings entra en Clouds. Con Rings, Clouds y una sola fuente de disparos se puede "
  "partir hacia atmósferas ambient infinitas: Rings actúa como una voz de modelado físico prácticamente completa, y "
  "Clouds como un delay/reverb granular complejo y brillante.")
h2("Por qué funciona tan bien")
ul("<b>Sonido con vida propia desde el primer segundo.</b> Un resonador no necesita envolventes ni filtros para sonar "
   "orgánico: cada golpe tiene ataque, cuerpo y una caída natural, rica en armónicos que evolucionan.",
   "<b>Material ideal para granular.</b> Los sonidos resonantes, afinados y con colas largas son perfectos para que Clouds "
   "los fragmente, los estire, los congele y los multiplique sin que se vuelvan ruido.",
   "<b>Dos módulos, dos dimensiones.</b> Rings define la <i>altura y el timbre</i>; Clouds, el <i>espacio y el tiempo</i>. "
   "Se complementan en lugar de pisarse.",
   "<b>Pocas conexiones, mucho resultado.</b> Con una secuencia o una fuente aleatoria (como Marbles) el patch "
   "evoluciona solo, algo perfecto para música generativa, instalaciones o directos tranquilos.",
   "<b>La misma mano detrás.</b> Ambos módulos comparten filosofía de diseño: controles de alto nivel, respuesta "
   "musical en todo el recorrido y un carácter sonoro reconocible.")
h2("Clásico, tópico y punto de partida")
p("El propio éxito del patch lo convirtió en tópico. Perfect Circuit lo describe como uno de los clichés más "
  "polarizadores del Eurorack y reconoce que se ha vuelto algo así como una broma, pero también que tiene mucho de "
  "verdad su permanencia: han visto a muchos músicos empezar su viaje modular con estos módulos como punto de "
  "partida. El canal Dawless Jamming llegó a convocar sesiones de modular meditativo bajo el lema «Rings into "
  "Clouds Forever», y todavía en 2025 se siguen publicando vídeos que explican por qué estos dos módulos "
  "siguen entre los más populares del Eurorack.")
p("Cuando Mutable Instruments cerró, entre los mensajes de despedida había uno que lo resumía todo: "
  "«Rings into Clouds, forever!». ResonatorIntoTexture "
  "nace de ese cariño: el patch más célebre de la marca, convertido en plugin, con todo lo necesario para ir más allá "
  "del tópico.")

# ======================================================================
h1("4. Instalación")
tbl([["Sistema", "Dónde copiar ResonatorIntoTexture.vst3", "Notas"],
     ["Linux", "~/.vst3/", "x86_64, glibc 2.35 o superior (Ubuntu 22.04+, Debian 12, Fedora, Arch)."],
     ["Windows", "C:\\Program Files\\Common Files\\VST3\\", "64 bits."],
     ["macOS", "~/Library/Audio/Plug-Ins/VST3/", "Universal (Apple Silicon e Intel), macOS 10.13 o superior. El plugin no está "
      "firmado por un desarrollador de Apple: si macOS lo bloquea, ejecuta en Terminal "
      "<font face='Mono'>xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ResonatorIntoTexture.vst3</font>"]],
    [0.13, 0.37, 0.50])
p("Después, vuelve a escanear los plugins en tu DAW (en REAPER: <i>Options → Preferences → Plug-ins → VST → Re-scan</i>). "
  "ResonatorIntoTexture es un efecto con entrada MIDI: ponlo en una pista con audio, o en una pista vacía si solo vas a "
  "usar el generador.")

# ======================================================================
h1("5. Visión general")
img("ui_gen.png", 1.0, "Los cinco paneles con el generador desplegado (patch «Voces en el pozo»).")
p("ResonatorIntoTexture es un pequeño sistema modular de cinco paneles. De izquierda a derecha:")
tbl([["Panel", "Basado en", "Función"],
     ["random sampler", "Marbles", "Genera puertas aleatorias (T1, T2, T3) y voltajes cuantizados (X1, X2, X3, Y)."],
     ["macro oscillator", "Plaits 1.2", "Oscilador con 24 modelos de síntesis."],
     ["in / out", "—", "Patches, nivel de entrada, generador, MIDI, mezcla y salida hacia el DAW."],
     ["resonator", "Rings", "Resonador de modelado físico: modal, cuerdas simpáticas, cuerda, FM…"],
     ["texture", "Clouds + Parasites + mrarm84", "Granular, stretch, delay, espectral, Oliverb, Resonestor, caja de ritmos y bender."]],
    [0.2, 0.25, 0.55])
p("El botón <b>GEN</b> del panel in/out muestra u oculta el random sampler y el macro oscillator. Sin ellos, "
  "ResonatorIntoTexture funciona como un efecto «Rings into Clouds» sobre el audio de la pista. Con ellos, se convierte "
  "en un generador autónomo que puede sonar solo.")
h2("Cada módulo a su frecuencia de muestreo original")
p("El DSP original se ejecuta sin modificar y a su frecuencia de muestreo nativa: random sampler, macro oscillator y "
  "resonator a 48 kHz, en bloques de 24 muestras; texture a 32 kHz, como el Clouds de hardware. Un remuestreador "
  "polifásico (sinc con ventana Kaiser) conecta esos dominios con la frecuencia del DAW. Así se conserva el carácter "
  "de cada módulo, incluido el brillo algo «oscuro» y granuloso de Clouds a 32 kHz.")

# ======================================================================
h1("6. Cables, voltajes y atenuadores")
h2("Conectar y desconectar")
ul("<b>Conectar:</b> arrastra desde una salida (los jacks sobre placa negra) hasta una entrada, o al revés.",
   "<b>Mover:</b> arrastra desde una entrada que ya tiene cable: lo despegas y lo llevas a otra entrada. Si lo sueltas "
   "en el vacío, se elimina.",
   "<b>Quitar:</b> clic derecho sobre un jack elimina todos sus cables. Clic derecho en el fondo del panel in/out: "
   "«Quitar todos los cables».",
   "Varias salidas conectadas a una misma entrada se suman. Una salida puede ir a muchas entradas.")
h2("Voltajes")
p("Las señales se comportan como en Eurorack: audio de ±5 V (equivale a ±1,0 digital), puertas de 0/5 V y notas en "
  "1 V/oct. Una puerta se considera alta por encima de 1 V.")
h2("Atenuversores de CV")
p("Cada entrada de control tiene encima un <b>atenuversor mini</b>, de −100 % a +100 %. A +100 %, 5 V recorren todo el "
  "recorrido del knob correspondiente; en las entradas de tono (FM y PITCH), +100 % equivale a 1 V/oct. Al conectar un "
  "cable a una entrada cuyo atenuversor está a cero, se abre automáticamente a +50 % (+100 % en FM y PITCH) para "
  "que el efecto se oiga enseguida.")
h2("Atenuadores de audio")
p("Las entradas de audio tienen un <b>atenuador de nivel</b> (de 0 a 100 %, por defecto 100 %): la entrada IN del "
  "resonador, IN L / IN R de texture y TO DAW L / R. Actúan sobre lo que entra, conectado por cable o por normalización.")
h2("Normalizaciones (qué ocurre sin cable)")
tbl([["Entrada", "Sin cable"],
     ["resonator IN", "Depende de EXCITER: audio de la pista, nada o excitador interno (más lo que entre por cable)."],
     ["resonator V/OCT y STRUM", "Las notas MIDI fijan el tono y rasguean. Sin MIDI: FREQUENCY y el detector de ataques de la señal."],
     ["oscillator V/OCT y TRIG", "Las notas MIDI tocan el oscilador. Sin MIDI: suena continuo (modo drone)."],
     ["texture IN L / R", "Salidas ODD / EVEN del resonador, cruzadas 60/40 para que ninguna voz quede en un solo canal."],
     ["TO DAW L / R", "Salida de texture."]],
    [0.3, 0.7])

# ======================================================================
h1("7. Panel in / out")
img("p_io.png", 0.24, "Panel in / out.")
tbl([["Control", "Función"],
     ["Selector de patch", "Fábrica y usuario. Flechas &lt; &gt;: anterior / siguiente. Clic en el nombre: menú de presets (ver capítulo 13)."],
     ["INPUT", "Ganancia de la entrada de la pista (−24 a +24 dB). Los LEDs muestran su nivel."],
     ["GEN", "Muestra u oculta el generador (random sampler + macro oscillator). Si no hay cables del generador, crea el "
      "cableado estándar: oscillator OUT → resonator IN, T1 → TRIG, X1 → V/OCT del oscilador, T3 → STRUM, X2 → V/OCT del resonador, Y → TIMBRE e Y → POS."],
     ["MIDI", "Muestra la tónica y el número de notas de la escala aprendida por MIDI."],
     ["MIX (+ CV)", "Mezcla entre la señal seca de la pista (compensada en latencia) y la procesada."],
     ["OUTPUT", "Ganancia de salida (−24 a +12 dB)."],
     ["TO DAW L / R", "Entradas de lo que suena en el DAW, con su atenuador de nivel."],
     ["Clic derecho", "Zoom de la interfaz (60–150 %) y «Quitar todos los cables»."]],
    [0.22, 0.78])

# ======================================================================
h1("8. random sampler (Marbles)")
img("p_marbles.png", 0.55, "Panel random sampler.")
h2("Sección T: puertas aleatorias")
tbl([["Control", "Función"],
     ["RATE", "Velocidad del reloj interno. Con SYNC o T CLK, multiplicador / divisor del reloj externo (centro = 1:1)."],
     ["BIAS", "Probabilidad relativa entre T1 y T3."],
     ["JITTER", "Irregularidad temporal: de reloj perfecto a «humano» y caótico."],
     ["T MODE", "Coin toss (Bernoulli complementario), Ratios, Drums, Independent, Divider, Three states, Markov."],
     ["T RANGE", "×0,25, ×1, ×4 sobre la velocidad."],
     ["Salidas T1 · T2 · T3", "T2 es el reloj maestro; T1 y T3 se reparten los pulsos según BIAS y el modelo."]],
    [0.22, 0.78])
h2("Déjà vu")
tbl([["Control", "Función"],
     ["DÉJÀ VU", "Probabilidad de reutilizar material pasado. En el centro (zona muerta), el bucle queda bloqueado y se repite exactamente."],
     ["LENGTH", "Longitud del bucle (1 a 16 pasos)."],
     ["T / X", "Activan el déjà vu para las puertas y/o para los voltajes."]],
    [0.22, 0.78])
h2("Sección X: voltajes aleatorios")
tbl([["Control", "Función"],
     ["SPREAD", "Anchura de la distribución: de casi constante a muy dispersa."],
     ["BIAS", "Desplaza la distribución hacia arriba o abajo."],
     ["STEPS", "A la izquierda, voltajes suavizados; en el centro, crudos; a la derecha, cuantizados a la escala, "
      "cada vez más limitados a los grados más importantes."],
     ["X RANGE", "+2 V, +5 V o ±5 V."],
     ["SCALE", "Clic: lista de 42 escalas en grupos, más «MIDI» (escala aprendida del teclado)."],
     ["ROOT", "Tónica para las escalas de fábrica."],
     ["Salidas X1 · X2 · X3 · Y", "X1–X3 se cuantizan y se transponen a la tónica; Y es una modulación lenta (0–5 V)."]],
    [0.22, 0.78])
h2("Escalas")
p("<b>Marbles:</b> Major, Minor, Pentatonic, Pelog, Raag Bhairav, Raag Shri (con los pesos originales). <b>Modos:</b> "
  "Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian, menor armónica, menor melódica, lidia dominante, "
  "frigia dominante. <b>Pentatónicas:</b> mayor, menor, blues, Hirajoshi, In sen, Iwato, Kumoi, Yo. <b>Simétricas:</b> "
  "cromática, tonos enteros, octatónicas H-W y W-H, aumentada, quintas, octavas. <b>Del mundo:</b> doble armónica, menor "
  "húngara, Raag Todi, Prometheus, enigmática. <b>Microtonales:</b> Slendro, Maqam Rast, Maqam Bayati, entonación justa, "
  "serie armónica.")
p("<b>Escala MIDI:</b> elige «MIDI» en SCALE y toca o mantén un acorde. Las notas mantenidas forman la escala y la más "
  "grave es la tónica. Se memoriza al soltar y se guarda con el proyecto.")
h2("Sincronía y reloj externo")
p("<b>SYNC</b> engancha el reloj al transporte del DAW; <b>DIV</b> elige la división (1/1, 1/2, 1/4, 1/8, 1/16 y "
  "tresillos). Con el transporte parado, el generador se detiene. La entrada <b>T CLK</b> acepta un reloj por cable y "
  "tiene prioridad sobre SYNC.")

# ======================================================================
h1("9. macro oscillator (Plaits 1.2)")
img("p_plaits.png", 0.4, "Panel macro oscillator.")
tbl([["Control", "Función"],
     ["MODEL ◀ ▶", "Elige uno de los 24 modelos. El color de los LEDs indica el banco: naranja (modelos nuevos de Plaits 1.2), verde y rojo."],
     ["FREQUENCY", "Afinación (±24 semitonos sobre C3)."],
     ["HARMONICS · TIMBRE · MORPH", "Los tres parámetros de timbre; su significado depende del modelo."],
     ["DECAY · LPG COLOUR", "Caída y carácter del low pass gate cuando el oscilador se dispara (TRIG)."],
     ["LEVEL", "Nivel de salida."],
     ["NOTES", "Cuantización de la nota final: libre, semitonos o la escala del random sampler."],
     ["Entradas", "FM, HARMO, TIMBRE, MORPH, DECAY, LPG, LEVEL, MODEL (con atenuversor), V/OCT y TRIG."],
     ["Salidas", "OUT y AUX (variante o subproducto de la señal principal)."]],
    [0.27, 0.73])
tbl([["Banco naranja", "Banco verde", "Banco rojo"],
     ["VA-VCF (analógico con filtro)", "Virtual analog", "Swarm (enjambre de osciladores)"],
     ["Phase distortion", "Waveshaping", "Filtered noise"],
     ["6-op FM I", "2-op FM", "Particles (ruido de partículas)"],
     ["6-op FM II", "Formant grain", "Inharmonic string"],
     ["6-op FM III", "Additive (armónicos)", "Modal resonator"],
     ["Wave terrain", "Wavetable", "Bass drum"],
     ["String machine", "Chords", "Snare drum"],
     ["Chiptune", "Speech (voz)", "Hi-hat"]],
    [0.333, 0.333, 0.334])

# ======================================================================
h1("10. resonator (Rings)")
img("p_rings.png", 0.5, "Panel resonator.")
tbl([["Control", "Función"],
     ["FREQUENCY", "Tónica del resonador (60 semitonos). Con V/OCT o MIDI se redondea a semitonos, como en el módulo."],
     ["STRUCTURE", "Modal: inarmonicidad y geometría del objeto. Cuerdas simpáticas: intervalos entre cuerdas. Cuerda: dispersión."],
     ["BRIGHTNESS", "Contenido armónico: de madera y nailon a cristal y acero."],
     ["DAMPING", "Duración de la resonancia."],
     ["POSITION", "Punto de excitación (como pulsar una cuerda cerca del puente o del centro)."],
     ["RESONATOR", "Modal, Sympathetic strings, Inharmonic string y los modos bonus: FM voice, Sympathetic quantized, String + reverb (LEDs rojos)."],
     ["POLYPHONY", "1, 2 o 4 voces. Cada STRUM pasa a la siguiente voz; las colas se solapan."],
     ["EXCITER", "Track + cables: audio de la pista más lo que entre por IN. Cables only: solo lo que entre por IN. Internal + cables: excitador interno, que golpea en cada STRUM."],
     ["NOTES", "Cuantización: libre, semitonos o la escala del random sampler."],
     ["ON", "Desactivado, el resonador deja pasar su entrada sin procesar."],
     ["Entradas", "FM, STRUC, BRIGHT, DAMP, POS (con atenuversor), STRUM, V/OCT e IN (con atenuador de nivel)."],
     ["Salidas", "ODD y EVEN: con polifonía 1, dos conjuntos de armónicos; con 2 o 4 voces, voces alternas."]],
    [0.2, 0.8])
p("<b>Detector de ataques.</b> Sin cable en STRUM y sin MIDI, el resonador rasguea solo cuando detecta un ataque en la "
  "señal de entrada, como el módulo original cuando solo se conecta IN.")

# ======================================================================
h1("11. texture (Clouds + Parasites + mrarm84)")
img("p_clouds.png", 0.55, "Panel texture en modo granular.")
h2("Controles comunes")
tbl([["Control", "Función"],
     ["MODE", "Clic: lista de los 8 modos. Los nombres de los knobs cambian con el modo."],
     ["QUALITY", "16 bits estéreo, 16 bits mono, 8 bits estéreo, 8 bits mono (buffer más largo y más «lo-fi»)."],
     ["FREEZE", "Congela el buffer (en Trig drums activa DUB)."],
     ["PERF FX", "Efectos de actuación en Trig drums y Data bender."],
     ["Opciones A / B", "Cambian según el modo: GRAINS, VOICE, KIT/AUDIO IN, ENGINE, CORRUPT."],
     ["ON", "Desactivado, texture deja pasar su entrada."],
     ["Entradas", "9 CV con atenuversor, FREEZE, TRIG, IN L / IN R (con atenuador de nivel)."],
     ["Salidas", "OUT L y OUT R."]],
    [0.2, 0.8])
h2("Los 8 modos y sus knobs")
tbl([["Modo", "POS", "SIZE", "PITCH", "DENS", "TEX", "BLEND", "SPREAD", "FDBK", "VERB"],
     ["Granular", "posición", "tamaño", "tono", "densidad", "textura", "seco/mojado", "estéreo", "feedback", "reverb"],
     ["Stretch", "posición", "ventana", "tono", "difusión", "filtro", "seco/mojado", "estéreo", "feedback", "reverb"],
     ["Looping", "retardo", "tamaño", "tono", "difusión", "filtro", "seco/mojado", "estéreo", "feedback", "reverb"],
     ["Spectral", "posición", "tamaño", "tono", "cuantiz.", "textura", "seco/mojado", "estéreo", "feedback", "reverb"],
     ["Oliverb", "pre-delay", "sala", "shift", "decay", "LP/HP", "seco/mojado", "difusión", "vel. mod.", "cant. mod."],
     ["Resonestor", "ráfaga", "acorde", "tono", "feedback", "filtro", "distorsión", "estéreo", "armonía", "disp. voces"],
     ["Trig drums", "char A", "contorno", "afinación", "char B", "filtro", "envío", "paneo", "eco fb", "cola rev."],
     ["Data bender", "tiempo", "repetic.", "bend", "break", "corrupt", "mezcla", "anchura", "regen", "prob. rev."]],
    [0.11] + [0.099] * 9)
h3("Granular")
p("El modo clásico de Clouds. DENSITY a las 12 detiene la generación de granos: a la izquierda los granos aparecen "
  "regularmente, a la derecha al azar. TRIG dispara un grano. La opción GRAINS (de Parasites) permite granos invertidos.")
h3("Stretch")
p("Time-stretcher / pitch-shifter WSOLA. Con PITCH a 0 y sin FREEZE el sonido apenas cambia: es lo esperado. Su magia "
  "aparece al cambiar PITCH sin cambiar la duración, o al congelar (FREEZE) y recorrer el buffer con POSITION.")
h3("Looping delay y Spectral")
p("Un delay en bucle con cambio de tono, y un procesador espectral (vocoder de fase) que cuantiza y difumina el espectro.")
h3("Oliverb")
p("La reverb creativa de Parasites: tamaño, decay, filtro LP/HP, difusión y una modulación que, con velocidades lentas "
  "y cantidades altas, produce sonidos sostenidos con altura propia. PITCH desplaza el tono de la cola; FREEZE da una "
  "reverb infinita.")
h3("Resonestor")
p("Un resonador de cuerdas y acordes de Parasites: TRIG excita una ráfaga, SIZE elige el acorde y PITCH la nota.")
h3("Trig drums")
p("Caja de ritmos de 8 voces. Los jacks de CV de texture pasan a ser disparos y sus rótulos cambian:")
tbl([["Jack", "POS", "SIZE", "PITCH", "TRIG", "DENS", "TEX", "BLEND", "FREEZE"],
     ["Voz", "BD bombo", "SD caja", "OH charles abierto", "CH charles cerrado (y reloj del eco)", "CP palmas", "MT multi (tom, conga, rim, cencerro)", "SA acorde", "SB resonador"]],
    [0.1] + [0.1125] * 8)
ul("<b>VOICE (KNOBS):</b> qué voz editan los knobs. Cada voz guarda sus valores; un knob no salta hasta que lo mueves.",
   "<b>KIT / AUDIO IN:</b> kit Analog (redondo) o Punch (con más ataque). El audio de entrada puede pasar (<i>Thru</i>), "
   "convertirse en el ruido de las voces (<i>Skin</i>) u oírse solo a través de los golpes (<i>Chop</i>).",
   "<b>FREEZE = DUB:</b> colas más largas y todo hacia el eco y la reverb.",
   "Para tocarla con el generador: T1 → POS (bombo), T3 → SIZE (caja), T2 → TRIG (charles).")
h3("Data bender")
p("Glitch de buffer: la entrada se graba en un buffer de un segundo y se reproduce en porciones repetidas, invertidas, "
  "aceleradas, rotas y corrompidas. ENGINE elige Traverse, Silence o Micro (cada uno también en reverso); CORRUPT elige "
  "Decimate, Dropout, Destroy, DJ filter o Vinyl. TRIG es el reloj, FREEZE congela y la entrada PITCH funciona como V/OCT "
  "(velocidad en Micro).")
h3("PERF FX")
p("En Trig drums y Data bender: Stutter, Tape stop, Filter sweep, Bitcrush, Wash, Reverse, Gate y Echo throw (u Octave "
  "down en Data bender), sincronizados con el pulso.")
p("<b>Nota:</b> los motores de Trig drums y Data bender se escribieron para un módulo físico y usan estado global. Por "
  "eso solo una instancia del plugin a la vez puede usar esos dos modos; si otra los está usando, aparece un aviso rojo. "
  "Los otros seis modos no tienen esa limitación.")

# ======================================================================
h1("12. MIDI")
ul("<b>Sin generador:</b> cada nota afina el resonador y lo rasguea; FREQUENCY transpone (en el centro, el tono es exacto).",
   "<b>Con generador:</b> si V/OCT y TRIG del oscilador no tienen cable, las notas MIDI lo tocan.",
   "<b>Escala MIDI:</b> con SCALE en «MIDI», las notas mantenidas definen la escala del random sampler y la cuantización NOTES.")

# ======================================================================
h1("13. Patches y presets de usuario")
h2("Patches de fábrica")
tbl([["Generativos (suenan solos)", "Efectos (para el audio de la pista)"],
     ["01 Lluvia de cristal · 02 Órgano de niebla · 03 Kalimba aleatoria · 04 Arpa eólica · 05 Pelog nocturno · "
      "06 Coro de partículas · 07 Voces en el pozo · 08 Enjambre · 09 Tambores de lata · 10 Cadena de Markov · "
      "11 Glaciar · 12 Campanas tibetanas · 13 Máquina de cuerdas · 14 Chiptune en la nieve · 15 Seis operadores · "
      "16 Terreno ondulado · 17 Ruido del mar",
      "18 Campana sobre la voz · 19 Cuerdas simpáticas · 20 Sitar eléctrico · 21 Espectro que respira · "
      "22 Eco de ruiseñor · 23 Gamelan de entrada · 24 Bajo lejano · 25 Respira"]],
    [0.6, 0.4])
p("Todos están nivelados alrededor de −22 dB RMS. También aparecen como programas VST3 del DAW.")
h2("Presets de usuario")
p("Clic en el nombre del patch: <b>Fábrica</b>, <b>Usuario</b> (los tuyos, en amarillo), <b>Guardar</b>, <b>Guardar "
  "como…</b>, <b>Cargar archivo…</b>, <b>Exportar a archivo…</b>, <b>Borrar</b> y <b>Abrir carpeta de presets</b>. Los "
  "presets son archivos <font face='Mono'>.ritpreset</font> (XML) con parámetros, cables, escala MIDI y los ajustes de "
  "cada voz de la caja de ritmos.")
tbl([["Sistema", "Carpeta de presets"],
     ["Linux", "~/.config/ResonatorIntoTexture/Presets/"],
     ["Windows", "%APPDATA%\\ResonatorIntoTexture\\Presets\\"],
     ["macOS", "~/Library/ResonatorIntoTexture/Presets/"]],
    [0.25, 0.75])

# ======================================================================
h1("14. Recetas")
h2("El Rings into Clouds clásico")
ul("Patch <b>Init</b>, pulsa <b>GEN</b>: aparece el cableado estándar.",
   "En el resonator: EXCITER en <i>Internal + cables</i>, RESONATOR en <i>Sympathetic strings</i>, POLYPHONY 4, DAMPING alto.",
   "En el random sampler: SCALE en una pentatónica, STEPS hacia la derecha, RATE lento, DÉJÀ VU cerca del centro.",
   "En texture: modo Granular, DENSITY a las 2, SIZE grande, REVERB y FEEDBACK a media altura.",
   "Ajusta MIX y OUTPUT. Pulsa play y deja que evolucione.")
h2("Un efecto con vida")
ul("Pon el plugin en una pista de guitarra o de voz, sin GEN.",
   "RESONATOR en Modal, STRUCTURE a la mitad, NOTES en <i>Scale</i> con la tónica de la canción.",
   "texture en Oliverb, DECAY alto, SHIFT +12 para un brillo octavado.")
h2("Ritmo granular")
ul("GEN activado, T MODE en <i>Drums</i>, texture en <i>Trig drums</i>.",
   "T1 → POS (bombo), T3 → SIZE (caja), T2 → TRIG (charles); KIT en Punch, PERF FX en Stutter.")

# ======================================================================
h1("15. Notas técnicas")
ul("Latencia reportada al host: unos 6 ms (los DAW la compensan).",
   "Consumo de CPU orientativo: entre el 1,5 % y el 5,5 % de un núcleo, según patch.",
   "Corrección respecto a versiones anteriores: el resonador se reiniciaba en cada bloque de audio y no llegaba a resonar; "
   "desde la 2.0 resuena con su caída natural.",
   "Los nombres de los paneles son funcionales (resonator, texture…) porque el README del repositorio de Mutable "
   "Instruments pide no usar su marca ni los nombres originales de los módulos en obras derivadas.")

# ======================================================================
h1("16. Créditos y licencias")
tbl([["Componente", "Autoría", "Licencia"],
     ["DSP de Rings, Plaits, Marbles, Clouds y stmlib", "Émilie Gillet / Mutable Instruments", "MIT"],
     ["Parasites (Oliverb, Resonestor…)", "Matthias Puech", "MIT"],
     ["Trig drums, Data bender, perf FX", "mrarm84", "MIT"],
     ["JUCE", "Raw Material Software", "AGPLv3 (licencia libre de JUCE 8)"],
     ["ResonatorIntoTexture (plugin, interfaz, cables, presets)", "Eloy Platas", "AGPLv3"]],
    [0.42, 0.33, 0.25])
small("Mutable Instruments es una marca registrada. Este proyecto no está afiliado ni respaldado por Mutable Instruments.")
h2("Fuentes")
small("Perfect Circuit, «Mutable Instruments Retrospective» y «Rings into Clouds» · Synthtopia, «Mutable Instruments R.I.P.» "
      "(dic. 2022) y «Mutable Instruments Rings Into Clouds, A Classic Modular Combo» (2025) · MusicRadar, cierre de Mutable "
      "Instruments (2022) · gearnews, «Mutable Instruments: No new modules» · Perfect Circuit, «Mutable Instruments Beads» · "
      "Reverb, «5 Fun Things to Do With Beads» · mutable-instruments.net/modules/rings · Resident Advisor (Plaits y Marbles, "
      "2018) · VCV Rack, Audible Instruments · Synthtopia, «Parasite brings new features to Clouds» · mqtthiqs.github.io/parasites · "
      "CDM, «Eurorack ambient radio» (2019) · MetaFilter, «Au revoir Mutable Instruments» (2022) · "
      "github.com/pichenettes/eurorack · github.com/mqtthiqs/parasites · github.com/mrarm84/customFWCloudsWithDrumMachineAndBender")

CONTENT = {
    "title": "ResonatorIntoTexture — Manual",
    "footer": "ResonatorIntoTexture 3.0 · Manual en castellano",
    "toc": "Contenido",
    "cover_sub": "Un homenaje modular a Mutable Instruments",
    "cover_line": "Marbles · Plaits · Rings · Clouds (Parasites) en un plugin VST3 con cables",
    "cover_ver": "Versión 3.0 · Manual de usuario · Eloy Platas",
    "blocks": B,
}
