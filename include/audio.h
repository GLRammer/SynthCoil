#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
// #include <fftw3.h> Unused atm and may be implemented elsewhere
#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#pragma once


// Define globals for audio
#define SAMPLE_RATE (48000)
#define NUM_CHANNELS (1)
#define PRINTF_S_FORMAT "%.8f"
typedef float SAMPLE;


class audio
{
private:
    SDL_AudioStream* stream;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;
    std::string errorString;
public:
    audio();
    /// @brief Select device for audio streams
    /// @param selected Index of selected device
    /// @return         Returns exit status
    bool selectDev(SDL_AudioDeviceID selected);
    /// @brief Start audio stream
    /// @return Return exit status
    bool startStream();
    /// @brief Check audio stream and cleanup if done
    /// @return Return exit status
    int catchStream(char* buff,int len);
    /// @brief Wipe the stream buffer. This discards data but allows fresh data to be collected.
    /// @return False on failure, check getErr() for details.
    bool clearBuff();
    std::string getErr();
    int available();
    float currVol();
    ~audio();
};
