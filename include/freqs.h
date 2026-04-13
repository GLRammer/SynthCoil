#pragma once
#include <fftw3.h>
#include "audio.h"
#include <vector>
#include <cmath>
#include <string>
#include <iostream>

#define FFTSZ 2048

/// @brief Class for fetching frequencies and magnitudes.
class freqHolder
{
private:
    float *freqbuff;
    fftwf_complex *out;
    fftwf_plan plan;
    std::vector<float> frequencies;
    std::vector<float> magnitudes;
    std::pair<float, float> peak;
    std::vector<std::pair<float, float>>topN;
    int topNum=5;
    float gain;

public:
    freqHolder();
    ~freqHolder();
    std::vector<float> getFrequencies();
    std::vector<float> getMagnitudes();
    std::pair<float, float> getPeak();
    std::vector<std::pair<float, float>> getTop();
    void setTop(int n);
    void freqGet(audio &in);
};
