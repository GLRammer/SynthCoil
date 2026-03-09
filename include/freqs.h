#pragma once
#include <fftw3.h>
#include <vector>
#include <cmath>
#include <cstring>
#include <iostream>

#define FFTSZ 2048

/// @brief Class to contain output for freqGet()
class freqHolder
{
public:
    std::vector<float> frequencies;
    std::vector<float> magnitudes;
    freqHolder(std::vector<float>, std::vector<float>);
};

/// @brief Use FFTW3 to convert raw audio to frequencies and magnitudes
/// @param in Vector of frames from audio input
/// @return A freqHolder object with a vector of strings and magnitudes
freqHolder freqGet(std::vector<float> in, int loc = 0);