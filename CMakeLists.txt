cmake_minimum_required(VERSION 3.22)

project(synthCoil
    VERSION 1.0
    LANGUAGES CXX
)

# Prevent accidental cross-toolchain compiler selection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER MATCHES "mingw|msys")
        message(FATAL_ERROR "Linux build detected but C++ compiler looks like MinGW/MSYS: ${CMAKE_CXX_COMPILER}")
    endif()
endif()

if(WIN32)
    if(CMAKE_CXX_COMPILER MATCHES "/usr/bin/")
        message(FATAL_ERROR "Windows build detected but compiler is a Linux one: ${CMAKE_CXX_COMPILER}")
    endif()
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(synthDemo
src/main.cpp
src/audio.cpp
src/freqs.cpp
)

# Grab portaudio
find_package(portaudio REQUIRED)

if(TARGET portaudio::portaudio)
    target_link_libraries(synthDemo PRIVATE portaudio::portaudio)
elseif(TARGET portaudio)
    target_link_libraries(synthDemo PRIVATE portaudio)
elseif(TARGET PortAudio::PortAudio)
    target_link_libraries(synthDemo PRIVATE PortAudio::PortAudio)
endif()


# Grab FFTW3
find_package(FFTW3 CONFIG QUIET)

if (FFTW3_FOUND)
  message(STATUS "Using FFTW via CMake config")
  set(FFTW_TARGET FFTW3::fftw3)    
else()
  # fallback to pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FFTW_PKG QUIET IMPORTED_TARGET fftw3)

  if (FFTW_PKG_FOUND)
    message(STATUS "Using FFTW via pkg-config")
    set(FFTW_TARGET PkgConfig::FFTW_PKG)
  else()
    message(FATAL_ERROR "FFTW not found.")
  endif()
endif()


target_link_libraries(synthDemo PRIVATE ${FFTW_TARGET})
