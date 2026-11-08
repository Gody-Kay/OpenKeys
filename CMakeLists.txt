cmake_minimum_required(VERSION 3.22)
project(OpenKeys VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JUCE is fetched only when asked (needs network + system audio dev packages).
option(OPENKEYS_WITH_JUCE "Fetch JUCE and build engine/app targets" OFF)

enable_testing()

# Shared interfaces (owned by lane A).
add_library(openkeys_common INTERFACE)
if(EXISTS "${CMAKE_SOURCE_DIR}/src/common")
  target_include_directories(openkeys_common INTERFACE "${CMAKE_SOURCE_DIR}/src/common")
endif()

if(OPENKEYS_WITH_JUCE)
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

# Lanes add their own targets from their own CMakeLists.txt, if present.
foreach(dir src/engine src/dsp src/synth src/ui src/app)
  if(EXISTS "${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt")
    add_subdirectory(${dir})
  endif()
endforeach()

# Phase 0 integration tests: toolchain smoke test + repo/schema checks.
add_executable(smoke_test tests/integration/smoke.cpp)
target_link_libraries(smoke_test PRIVATE openkeys_common)
add_test(NAME toolchain_smoke COMMAND smoke_test)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME repo_layout_and_schema
           COMMAND ${Python3_EXECUTABLE} -I
                   ${CMAKE_SOURCE_DIR}/tests/integration/test_repo.py
                   ${CMAKE_SOURCE_DIR})
endif()

# Preset validation (tools/check_presets.py). Exit 77 = skipped.
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME presets_valid COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_presets.py)
  set_tests_properties(presets_valid PROPERTIES SKIP_RETURN_CODE 77)
endif()
