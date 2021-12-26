cmake_minimum_required(VERSION 3.5)

project(OpenPianoCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenPianoCore
    main.cpp
    dr_wav.h
    dr_wav.cpp
    array_helpers.h
    array_helpers.cpp
    string_hammer.h
    )

IF (NOT WIN32)
  target_link_libraries(OpenPianoCore m)
ENDIF()
