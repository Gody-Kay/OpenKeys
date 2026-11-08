# Lane B UI, Phase 0 draft. Built only with OPENKEYS_WITH_JUCE=ON.
if(OPENKEYS_WITH_JUCE)
  juce_add_gui_app(openkeys_app PRODUCT_NAME "OpenKeys")
  target_sources(openkeys_app PRIVATE Main.cpp PerformanceScreen.h)
  target_compile_definitions(openkeys_app PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
  target_link_libraries(openkeys_app PRIVATE juce::juce_gui_basics juce::juce_recommended_config_flags)
endif()
