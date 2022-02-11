cmake_minimum_required(VERSION 3.5)

project(OpenPianoCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenPianoCore
    Source/main.cpp
    Source/dr_wav.h
    Source/dr_wav.cpp
    Source/array_helpers.h
    Source/array_helpers.cpp
    Source/string_hammer.h
    Source/piano.h
    )

IF (NOT WIN32)
  target_link_libraries(OpenPianoCore m)
ENDIF()

find_package (Threads REQUIRED)
target_link_libraries(OpenPianoCore Threads::Threads)
