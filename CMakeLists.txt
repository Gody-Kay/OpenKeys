cmake_minimum_required(VERSION 3.22)
project(OpenKeys VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OPENKEYS_BUILD_TESTS "Build and register tests" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(OPENKEYS_BUILD_TESTS)
  enable_testing()
endif()

# JUCE is fetched only once a lane has added code that needs it (keeps early CI fast).
if(EXISTS "${CMAKE_SOURCE_DIR}/src/engine/CMakeLists.txt" OR EXISTS "${CMAKE_SOURCE_DIR}/src/app/CMakeLists.txt")
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW ON)
  FetchContent_MakeAvailable(JUCE)
endif()

# Each lane owns its own CMakeLists.txt. We add a folder only when it has one.
foreach(dir src/common src/dsp src/synth src/engine src/ui src/app)
  if(EXISTS "${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt")
    add_subdirectory(${dir})
  endif()
endforeach()

if(OPENKEYS_BUILD_TESTS)
  foreach(dir tests/engine tests/ui)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt")
      add_subdirectory(${dir})
    endif()
  endforeach()

  # Lane C integration checks (Python)
  find_package(Python3 COMPONENTS Interpreter)
  if(Python3_FOUND)
    add_test(NAME schema_valid
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/integration/validate_schema.py
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME licenses_logged
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_licenses.py
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif()
endif()
