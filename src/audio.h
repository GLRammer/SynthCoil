#include <portaudio.h>
// #include <fftw3.h> Unused atm and may be implemented elsewhere
#include <string>
#include <cmath>
#include <vector>

typedef float SAMPLE;

/// @brief Data structure for parsing audio
typedef struct
{
    int frameIndex; /* Index into sample array. */
    int maxFrameIndex;
    std::vector<SAMPLE> recordedSamples;
} paData;

/// @brief Spin up the PA stream and callback and automatically close stream when it ends
/// @param numSec       duration of audio in seconds
/// @param data         data pointer
/// @param input        input device
/// @param errorString  string for error catching and output
/// @return             returns -1 in case of error
int spinUp(float numSec, paData *data, PaDeviceIndex input, std::string *errorString);
