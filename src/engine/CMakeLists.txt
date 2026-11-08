add_library(openkeys_engine STATIC PartEngine.cpp)
target_include_directories(openkeys_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(openkeys_engine PUBLIC openkeys_common openkeys_dsp)

# Offline tests live in tests/engine (lane A); registered here so root CMake stays untouched.
add_executable(engine_render_test ${CMAKE_SOURCE_DIR}/tests/engine/render_test.cpp)
target_link_libraries(engine_render_test PRIVATE openkeys_engine)
add_test(NAME engine_render COMMAND engine_render_test ${CMAKE_BINARY_DIR}/render_test.wav)

add_executable(engine_parts_test ${CMAKE_SOURCE_DIR}/tests/engine/parts_test.cpp)
target_link_libraries(engine_parts_test PRIVATE openkeys_engine)
add_test(NAME engine_parts COMMAND engine_parts_test)

add_executable(engine_effects_test ${CMAKE_SOURCE_DIR}/tests/engine/effects_test.cpp)
target_link_libraries(engine_effects_test PRIVATE openkeys_engine)
add_test(NAME engine_effects COMMAND engine_effects_test)

# Phase 0 console app: real MIDI keyboard -> engine -> sound card (needs JUCE).
if(OPENKEYS_WITH_JUCE)
  option(OPENKEYS_NO_AUDIO_BACKENDS "Compile-check only: disable ALSA/JACK (no sound)" OFF)
  juce_add_console_app(openkeys_phase0 PRODUCT_NAME "OpenKeys Phase0")
  target_sources(openkeys_phase0 PRIVATE Phase0Audio.cpp)
  target_compile_definitions(openkeys_phase0 PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
  if(OPENKEYS_NO_AUDIO_BACKENDS)
    target_compile_definitions(openkeys_phase0 PRIVATE JUCE_ALSA=0 JUCE_JACK=0)
  elseif(UNIX AND NOT APPLE)
    target_compile_definitions(openkeys_phase0 PRIVATE JUCE_ALSA=1 JUCE_JACK=1)
  endif()
  target_link_libraries(openkeys_phase0 PRIVATE
    openkeys_engine juce::juce_audio_devices juce::juce_events
    juce::juce_recommended_config_flags)
endif()
