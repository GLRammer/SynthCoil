#include <portaudio.h>
// #include <fftw3.h> Unused atm and may be implemented elsewhere
#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#pragma once


// Define globals for portaudio
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS (1)
#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SILENCE (0.0f)
#define PRINTF_S_FORMAT "%.8f"
typedef float SAMPLE;

/// @brief Data structure for parsing audio
typedef struct
{
    int frameIndex; /* Index into sample array. */
    int maxFrameIndex;
    std::vector<SAMPLE> recordedSamples;
} paData;

class audio
{
private:
    PaStreamParameters inParam;
    PaStream *stream;
    std::string errorString;
    PaError err;
    paData *data;
public:
    audio();
    // Callback function for portaudio. Called as interrupt. Mostly pulled from recording sample code.
    static int MyStreamCallback(const void *input,
                            void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);
    /// @brief Initialize portaudio setup
    /// @param numSec   Duration of audio in seconds
    /// @param indata     Data pointer
    /// @return         Returns exit status
    int spinUp(float numSec, paData *indata);
    /// @brief Select device for audio streams
    /// @param selected Index of selected device
    /// @return         Returns exit status
    int selectDev(PaDeviceIndex selected);
    /// @brief Start portaudio stream
    /// @return Return exit status
    int startStream();
    /// @brief Check portaudio stream and cleanup if done
    /// @return Return exit status
    int catchStream();
    std::string getErr();
    ~audio();
};
