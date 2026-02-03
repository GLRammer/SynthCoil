#include "audio.h"
#include <string>

int main(int argc, char** argv)
{
    paData data;
    std::string errorstr = "";
    if(spinUp(5.0, &data,Pa_GetDefaultInputDevice(),&errorstr)){
        SAMPLE max=0;
        for(int i=0;i<data.recordedSamples.size();i++){
            if(data.recordedSamples.at(i)>max)
                max = data.recordedSamples.at(i);
        }
        printf("Max amp of %f",max);
    } else{
        printf("%s",errorstr);
    }
    return 0;
}
