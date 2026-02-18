#include "audio.h"
// #include "freqs.h"
#include "main.h"
#include <string>
#include <iostream>

int main()
{
    paData data;
    std::string errorstr = "";
    if(spinUp(5.0, data,errorstr)==0){
        SAMPLE max=0;
        for(int i=0;i<(int)data.recordedSamples.size();i++){
            if(data.recordedSamples[i]>max)
                max = data.recordedSamples[i];
        }
        std::cout<< "Max amp of " << max << std::endl;
    } else{
        std::cout<<errorstr<<std::endl;
    }
    return 0;
}
