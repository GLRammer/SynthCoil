#include "audio.h"

audio::audio(){
    errorString="";
    spec.channels=NUM_CHANNELS;
    spec.format=SDL_AUDIO_F32;
    spec.freq=SAMPLE_RATE;
}


audio::~audio()
{
    SDL_DestroyAudioStream(stream);
}

int audio::selectDev(SDL_AudioDeviceID selected){
    dev=selected;
    return 0;
}

int audio::startStream(){

    stream=SDL_OpenAudioDeviceStream(dev,&spec,NULL,NULL);   
    if (stream==NULL){
        errorString=SDL_GetError();
        return -1;
    }
    if(!SDL_ResumeAudioStreamDevice(stream)){
        errorString=SDL_GetError();
        return -1;
    }
    return 0;
}

int audio::catchStream(char* buff,int len){
    int temp=SDL_GetAudioStreamData(stream,buff,len);
    if (temp==-1){
        errorString=SDL_GetError();
        return -1;
    }
    return temp;
}


int audio::available(){
    // if(SDL_AudioStreamDevicePaused(stream)){
    //     if(!SDL_ResumeAudioStreamDevice(stream)){
    //         errorString=SDL_GetError();
    //         return -1;
    //     }
    // }
    int avail=SDL_GetAudioStreamAvailable(stream);
    if (avail<=0){
        if(avail==-1)
            errorString=SDL_GetError();
        return -1;
    }
    return avail;
}

float audio::currVol(){
    if(available()>=0){
        float buffer=0;
        SDL_GetAudioStreamData(stream,&buffer,sizeof(float));
        return buffer;
    }
    return 0.0;
}

std::string audio::getErr(){
    return errorString;
}

