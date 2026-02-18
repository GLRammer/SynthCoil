#include "audio.h"
#include <portaudio.h>

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
    // SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    // (void)output; /* Prevent unused variable warnings. */
    // (void)timeInfo;
    // (void)statusFlags;
    // (void)userData;

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
            data->recordedSamples.push_back(SAMPLE_SILENCE); /* left */
            if (NUM_CHANNELS == 2)
            data->recordedSamples.push_back(SAMPLE_SILENCE); /* right */
        }
    }
    else
    {
        for (i = 0; i < framesToCalc; i++)
        {
            data->recordedSamples.push_back(*rptr++); /* left */
            // std::cout<<" " << wptr+i << ":"<<*rptr+i;
            if (NUM_CHANNELS == 2)
            data->recordedSamples.push_back(*rptr++); /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

int spinUp(float numSec, paData &data, std::string &errorString)
{
    // Portaudio setup
    PaStreamParameters inParam;
    PaStream *stream;
    PaError err = paNoError;
    // int i;
    int totalFrames;
    int numSamples;
    // int numBytes;
    // SAMPLE max, val;
    // double average;

    // Move data initializing and testing to main or whenever data is initially called
    // Unless spinUp is to be called multiple times
    data.maxFrameIndex = totalFrames = numSec * SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    data.recordedSamples.reserve(numSamples);

    data.recordedSamples.clear();
    // END data initialization and testing

    err = Pa_Initialize();
    if (err != paNoError)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }

    // printf("\n\n%d\n\n", Pa_GetDeviceCount());

    int devCnt = Pa_GetDeviceCount();
    if (devCnt == 0)
    {
        errorString = "No devices found.\n";
        return -1;
    }
    // std::cout << devCnt << " devices found.\n";

    std::cout << "Would you like to use this device: " << Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->name << " from " << Pa_GetHostApiInfo(Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->hostApi)->name << std::endl;
    std::string tmpstr;
    std::cin >> tmpstr;
    if (tmpstr == "n" || tmpstr == "no")
    {
        for (int i = 0; i < devCnt; i++)
        {
            std::cout << "Would you like to use this device: " << Pa_GetDeviceInfo(i)->name << " from " << Pa_GetHostApiInfo(Pa_GetDeviceInfo(i)->hostApi)->name << std::endl;
            std::cin >> tmpstr;
            if (tmpstr == "y" || tmpstr == "yes")
            {
                devCnt = i;
                std::cout << "Using this device: " << Pa_GetDeviceInfo(devCnt)->name << std::endl;
                break;
            }
        }
    }else{
        devCnt = Pa_GetDefaultInputDevice();
    }

    // Move device testing to earlier
    PaDeviceIndex input = devCnt;
    if (input == paNoDevice)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }
    inParam.device = input;
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
        &data);
    if (err != paNoError)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }

    while ((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(1000);
        std::cout << "index = " << data.frameIndex << std::endl;
        std::cout << "volume = " << data.recordedSamples[data.frameIndex-1] << std::endl;
    }
    if (err < 0)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        errorString = Pa_GetErrorText(err);
        return -1;
    }
    // std::cout << data.recordedSamples.size() << std::endl;
    return 0;
}
