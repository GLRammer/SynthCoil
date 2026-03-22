#pragma once
#include <fftw3.h>
#include "audio.h"
#include <vector>
#include <cmath>
#include <string>
#include <iostream>

#define FFTSZ 2048

/// @brief Class to contain output for freqGet()
class freqHolder
{
private:
    float* freqbuff;
    fftwf_complex* out;
    fftwf_plan plan;
    std::pair<float,float> peak;
public:
    freqHolder();
    ~freqHolder();
    std::vector<float> frequencies;
    std::vector<float> magnitudes;
    std::pair<float,float> getPeak();
    void freqGet(audio& in);
};

