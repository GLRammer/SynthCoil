#pragma once
#include <complex.h>
#include <fftw3.h>
#include "audio.h"
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <list>

// Number of samples to grab for audio and maximum index for frequencies and magnitudes
constexpr float FFTSZ = 2048.0;
constexpr float OUTSZ = (FFTSZ / 2.0) + 1.0;

// Upper range of human hearing in Hz accordning to Rosen, Stuart(2011) Signals for Speech and Hearing
constexpr float HEARINGMAX = 20000.0;

/// @brief Class for fetching frequencies and magnitudes.
class freqHolder
{
private:
    // Buffers for audio processing
    std::vector<float> freqbuff;
    std::vector<std::complex<float>> out;
    std::list<std::vector<std::complex<float>>> outFrames;

    // FFTW processing object
    fftwf_plan plan;

    // Storage for all processed data
    std::vector<float> frequencies;
    std::vector<float> magnitudes;

    // Loudest frequencies
    std::list<std::pair<float, float>> peak;
    std::vector<std::pair<float, float>> topN;

    // minimum and maximum frequency in Hz
    float min = 20, max = HEARINGMAX;

    // Number of frequencies to pass to rendering
    int topNum = 5;

    // Audio gain (normalize magnitude so that gain=1)
    float gain = 0.9;

    // Buffer frame count
    int tweenFrames = 5;

public:
    freqHolder();
    ~freqHolder();
    // Getters
    const std::vector<float> getFrequencies();
    const std::vector<float> getMagnitudes();
    const std::pair<float, float> getPeak();
    const std::vector<std::pair<float, float>> getTop();
    const float getGain();
    const int getFrames();
    void getMinMax(float &inmin, float &inmax);

    bool autoGain = true;
    // Setters
    void setMinMax(const float newmin, const float newmax);
    void setGain(const float newgain);
    void setFrames(const int newframes);
    void setTop(const int n);

    /// @brief Retrieve data from audio input
    void freqGet(audio &in);
};

/// @brief Convert Hz to human perception using Mel scale.
/// @param input    target frequency in Hz
/// @param min      minimum frequency range
/// @param max      maximum frequency range
/// @return         normalized frequency [0-1]
float melConv(float input, float min, float max);

/// @brief Helper function for Mel scale
/// @param freq     frequency to convert in Hz
/// @return         frequency according to Mel scale
float melHelp(float freq);