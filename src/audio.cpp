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

int audio::selectDev(SDL_AudioDeviceID selected=SDL_AUDIO_DEVICE_DEFAULT_RECORDING){
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

int audio::catchStream(void* buff,int len){
    int running=0;
    void* tempbuf=malloc(len);
    while(running<len){
        int temp=SDL_GetAudioStreamData(stream,tempbuf,len);
        if (temp==-1){
            errorString=SDL_GetError();
            free(tempbuf);
            return -1;
        }
        running+=temp;
    }
    memcpy(buff,tempbuf,len);
    free(tempbuf);
    return 0;
}


int audio::available(){
    if (SDL_GetAudioStreamAvailable(stream)<=0){
        return -1;
    }
    return 0;
}

float audio::currVol(){
    if(available()==0){
        float buffer=0;
        SDL_GetAudioStreamData(stream,&buffer,sizeof(float));
        return buffer;
    }
    return 0.0;
}

std::string audio::getErr(){
    return errorString;
}

