add_library(openkeys_dsp STATIC Reverb.cpp StereoDelay.cpp Chorus.cpp)
target_include_directories(openkeys_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
