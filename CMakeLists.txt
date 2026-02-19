cmake_minimum_required(VERSION 3.22)

project(synthCoil
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(synthDemo
src/main.cpp
src/audio.cpp
# src/freqs.cpp
)

# Grab portaudio
add_subdirectory("vendor/portaudio" "vendor/portaudio" EXCLUDE_FROM_ALL)

target_link_libraries(synthDemo PRIVATE portaudio)


# Grab FFTW3
# add_subdirectory("vendor/fftw3" "vendor/fftw3" EXCLUDE_FROM_ALL)

# target_link_libraries(synthDemo PRIVATE fftw3)


# target_link_libraries(synthDemo PRIVATE ${FFTW_TARGET})
