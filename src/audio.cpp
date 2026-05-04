#include "audio.h"

audio::audio()
{
    errorString = "";
    spec.channels = NUM_CHANNELS;
    spec.format = SDL_AUDIO_F32;
    spec.freq = SAMPLE_RATE;
    dev = SDL_AUDIO_DEVICE_DEFAULT_RECORDING;
    stream = nullptr;
}

audio::~audio()
{
    if (stream != nullptr)
        SDL_DestroyAudioStream(stream);
}

bool audio::selectDev(SDL_AudioDeviceID selected)
{
    // Check given device properties
    if (SDL_IsAudioDevicePlayback(selected))
    {
        errorString = "Selected device is not a recording device.";
        return false;
    }
    // Set device to selection
    dev = selected;
    return true;
}

bool audio::startStream()
{
    // Open stream with stored specs
    stream = SDL_OpenAudioDeviceStream(dev, &spec, NULL, NULL);
    if (stream == NULL)
    {
        // Set error and return failure
        errorString = SDL_GetError();
        return false;
    }
    if (!SDL_ResumeAudioStreamDevice(stream))
    {
        // If stream refuses to start, set error, destroy stream, and return failure
        errorString = SDL_GetError();
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
        return false;
    }
    return true;
}

int audio::catchStream(char *buff, int len)
{
    int total = 0;
    int temp = SDL_GetAudioStreamData(stream, buff, len);
    // Loop data caching until desired amount is recieved or until error
    while (total < len && temp != -1)
    {
        total += temp;
        temp = SDL_GetAudioStreamData(stream, buff + total, len - total);
    }
    // Set error and return failure
    if (temp == -1)
    {
        errorString = SDL_GetError();
        return -1;
    }
    return total;
}

bool audio::clearBuff()
{
    if (!SDL_ClearAudioStream(stream))
    {
        errorString = SDL_GetError();
        return false;
    }
    return true;
}

int audio::available()
{
    int avail = SDL_GetAudioStreamAvailable(stream);
    if (avail <= 0)
    {
        if (avail == -1)
            errorString = SDL_GetError();
        return avail;
    }
    return avail;
}

std::string audio::getErr()
{
    return errorString;
}
