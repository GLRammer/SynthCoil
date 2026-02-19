#pragma once
#include <fftw3.h>
#include <vector>
#include <cmath>

#define FFTSZ 2048

/// @brief Class to contain output for freqGet()
class freqHolder{
    public:
        std::vector<double> frequencies;
        std::vector<double> magnitudes;
        freqHolder();
        freqHolder(std::vector<double>,std::vector<double>);
};



/// @brief Use FFTW3 to convert raw audio to frequencies and magnitudes
/// @param in Vector of frames from audio input
/// @return A freqHolder object with a vector of strings and magnitudes
freqHolder freqGet(std::vector<double> in);