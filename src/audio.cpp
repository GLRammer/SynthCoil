#include "audio.h"

// Define globals for portaudio
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS (1)
#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SILENCE (0.0f)
#define PRINTF_S_FORMAT "%.8f"

// Callback function for portaudio. Called as interrupt. Mostly pulled from recording sample code.
static int MyStreamCallback(const void *input,
                            void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    paData *data = (paData *)userData;
    const SAMPLE *rptr = (const SAMPLE *)input;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void)output; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (framesLeft < frameCount)
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = frameCount;
        finished = paContinue;
    }

    if (input == NULL)
    {
        for (i = 0; i < framesToCalc; i++)
        {
            *wptr++ = SAMPLE_SILENCE; /* left */
            if (NUM_CHANNELS == 2)
                *wptr++ = SAMPLE_SILENCE; /* right */
        }
    }
    else
    {
        for (i = 0; i < framesToCalc; i++)
        {
            *wptr++ = *rptr++; /* left */
            if (NUM_CHANNELS == 2)
                *wptr++ = *rptr++; /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

int spinUp(float numSec, paData *data, PaDeviceIndex input, std::string *errorString)
{
    // Portaudio setup
    PaStreamParameters inParam;
    PaStream *stream;
    PaError err = paNoError;
    int i;
    int totalFrames;
    int numSamples;
    int numBytes;
    // SAMPLE max, val;
    // double average;

    // Move data initializing and testing to main or whenever data is initially called
    // Unless spinUp is to be called multiple times
    data->maxFrameIndex = totalFrames = numSec * SAMPLE_RATE;
    data->frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data->recordedSamples = (SAMPLE *)malloc(numBytes);

    if (data->recordedSamples == NULL)
    {
        *errorString = "Could not allocate record array.";
        return -1;
    }
    for (i = 0; i < numSamples; i++)
        data->recordedSamples[i] = 0;
    //END data initialization and testing

    err = Pa_Initialize();
    if (err != paNoError)
    {
        *errorString = "Initialize Failure.";
        return -1;
    }

    // Move device testing to earlier
    if (input == paNoDevice)
    {
        *errorString = "No device given.";
        return -1;
    }
    inParam.channelCount = NUM_CHANNELS; 
    inParam.sampleFormat = PA_SAMPLE_TYPE;
    inParam.suggestedLatency = Pa_GetDeviceInfo(inParam.device)->defaultLowInputLatency;
    inParam.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        &inParam,
        NULL, /* &outputParameters, */
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff, /* we won't output out of range samples so don't bother clipping them */
        MyStreamCallback,
        data);
    if (err != paNoError)
    {
        *errorString = "Failed to open stream.";
        return -1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        *errorString = "Failed to start stream.";
        return -1;
    }

    while ((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(1000);
        printf("index = %d\n", data->frameIndex);
        fflush(stdout);
    }
    if (err < 0)
    {
        *errorString = "Stream crash.";
        return -1;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        *errorString = "Failed to close stream.";
        return -1;
    }
    return 0;
}

