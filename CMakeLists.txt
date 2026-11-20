cmake_minimum_required(VERSION 3.22)
project(ResonatorIntoTexture VERSION 3.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
endif()
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE" CACHE PATH "JUCE source dir")
set(EURORACK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/eurorack" CACHE PATH "pichenettes/eurorack dir")
set(CLOUDS_FW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/clouds_fw")

set(RIC_FORMATS "VST3" CACHE STRING "Formatos JUCE")
add_subdirectory(${JUCE_DIR} JUCE EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# DSP original de Émilie Gillet (código STM32 bajo licencia MIT)
# ---------------------------------------------------------------------------
file(GLOB PLAITS_SRC
    ${EURORACK_DIR}/plaits/dsp/*.cc
    ${EURORACK_DIR}/plaits/dsp/engine/*.cc
    ${EURORACK_DIR}/plaits/dsp/engine2/*.cc
    ${EURORACK_DIR}/plaits/dsp/speech/*.cc
    ${EURORACK_DIR}/plaits/dsp/physical_modelling/*.cc
    ${EURORACK_DIR}/plaits/dsp/chords/*.cc
    ${EURORACK_DIR}/plaits/dsp/fm/*.cc
    ${EURORACK_DIR}/plaits/resources.cc)

set(MI_SRC
    ${EURORACK_DIR}/stmlib/dsp/units.cc
    ${EURORACK_DIR}/stmlib/dsp/atan.cc
    ${EURORACK_DIR}/stmlib/utils/random.cc
    # Rings
    ${EURORACK_DIR}/rings/dsp/fm_voice.cc
    ${EURORACK_DIR}/rings/dsp/part.cc
    ${EURORACK_DIR}/rings/dsp/string_synth_part.cc
    ${EURORACK_DIR}/rings/dsp/string.cc
    ${EURORACK_DIR}/rings/dsp/resonator.cc
    ${EURORACK_DIR}/rings/resources.cc
    # Texture: DSP de Clouds con Parasites (mqtthiqs) vía mrarm84 (MIT)
    ${CLOUDS_FW_DIR}/clouds/dsp/correlator.cc
    ${CLOUDS_FW_DIR}/clouds/dsp/granular_processor.cc
    ${CLOUDS_FW_DIR}/clouds/dsp/mu_law.cc
    ${CLOUDS_FW_DIR}/clouds/dsp/pvoc/frame_transformation.cc
    ${CLOUDS_FW_DIR}/clouds/dsp/pvoc/phase_vocoder.cc
    ${CLOUDS_FW_DIR}/clouds/dsp/pvoc/stft.cc
    ${CLOUDS_FW_DIR}/clouds/resources.cc
    # Marbles
    ${EURORACK_DIR}/marbles/random/t_generator.cc
    ${EURORACK_DIR}/marbles/random/x_y_generator.cc
    ${EURORACK_DIR}/marbles/random/output_channel.cc
    ${EURORACK_DIR}/marbles/random/lag_processor.cc
    ${EURORACK_DIR}/marbles/random/quantizer.cc
    ${EURORACK_DIR}/marbles/random/discrete_distribution_quantizer.cc
    ${EURORACK_DIR}/marbles/ramp/ramp_extractor.cc
    ${EURORACK_DIR}/marbles/resources.cc
    ${PLAITS_SRC})

add_library(mi_dsp STATIC ${MI_SRC})
target_include_directories(mi_dsp PUBLIC ${CLOUDS_FW_DIR} ${EURORACK_DIR})
target_compile_definitions(mi_dsp PUBLIC TEST=1)
if(MSVC)
  # MSVC / clang-cl
  target_compile_options(mi_dsp PRIVATE /O2 /fp:fast /w /FIcstdio -Wno-c++11-narrowing)
  target_compile_definitions(mi_dsp PUBLIC _USE_MATH_DEFINES NOMINMAX)
else()
  target_compile_options(mi_dsp PRIVATE -O3 -ffast-math -fno-finite-math-only -w -include cstdio)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(mi_dsp PRIVATE -Wno-c++11-narrowing)
  endif()
endif()

# ---------------------------------------------------------------------------
# Plugin
# ---------------------------------------------------------------------------
juce_add_plugin(ResonatorIntoTexture
    COMPANY_NAME "Eloy Platas"
    BUNDLE_ID "com.eloyplatas.resonatorintotexture"
    PLUGIN_MANUFACTURER_CODE Eplt
    PLUGIN_CODE ReTx
    PRODUCT_NAME "ResonatorIntoTexture"
    FORMATS ${RIC_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES Fx Generator
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(ResonatorIntoTexture PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Engine.cpp
    Source/TextureModes.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Compatibilidad con glibc 2.35 y libstdc++ estática (solo Linux)
  target_sources(ResonatorIntoTexture PRIVATE Source/compat_glibc.c)
endif()

target_compile_definitions(ResonatorIntoTexture PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(ResonatorIntoTexture
    PRIVATE mi_dsp juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)

# Los recursos de cada módulo redefinen macros LUT_*; el orden de inclusión es correcto.
if(MSVC)
  set_source_files_properties(Source/Engine.cpp Source/TextureModes.cpp PROPERTIES COMPILE_OPTIONS "/w")
else()
  set_source_files_properties(Source/Engine.cpp Source/TextureModes.cpp PROPERTIES COMPILE_OPTIONS "-w")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_source_files_properties(Source/compat_glibc.c PROPERTIES COMPILE_OPTIONS "-U_GNU_SOURCE;-std=c11;-fno-builtin;-fno-lto;-w")
  # Sin dependencia de una libstdc++ reciente en el sistema del usuario.
  target_link_options(ResonatorIntoTexture_VST3 PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Herramienta de captura de la interfaz (solo desarrollo)
if(RIC_BUILD_SNAPSHOT)
  juce_add_console_app(Snapshot PRODUCT_NAME "Snapshot")
  target_sources(Snapshot PRIVATE test/snapshot.cpp)
  target_compile_definitions(Snapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1)
  target_link_libraries(Snapshot PRIVATE ResonatorIntoTexture mi_dsp juce::juce_audio_utils juce::juce_dsp)
  juce_add_console_app(PresetTest PRODUCT_NAME "PresetTest")
  target_sources(PresetTest PRIVATE test/preset_test.cpp)
  target_compile_definitions(PresetTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
  target_link_libraries(PresetTest PRIVATE ResonatorIntoTexture mi_dsp juce::juce_audio_utils juce::juce_dsp)
  juce_add_console_app(PortDebug PRODUCT_NAME "PortDebug")
  target_sources(PortDebug PRIVATE test/port_debug.cpp)
  target_link_libraries(PortDebug PRIVATE ResonatorIntoTexture mi_dsp juce::juce_audio_utils juce::juce_dsp)
endif()
