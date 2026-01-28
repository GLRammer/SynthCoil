#include <portaudio.h>
#include <fftw3.h>
#include <string>
#include <cmath>

/// @brief Data structure for parsing audio
typedef struct paData;

/// @brief Spin up the PA stream and callback
/// @param numSec       duration of audio in seconds
/// @param data         data pointer
/// @param input        input device
/// @param errorString  string for error catching and output
/// @return             returns -1 in case of error
int spinUp(float numSec, paData *data, PaDeviceIndex input, std::string errorString);
