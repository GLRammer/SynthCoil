#include "audio.h"
#include "freqs.h"
#include "main.h"
#include <string>
#include <iostream>

int main()
{
    paData data;
    std::string errorstr = "";
    if(spinUp(5.0, data,errorstr)){
        SAMPLE max=0;
        for(int i=0;i<(int)data.recordedSamples.size();i++){
            if(data.recordedSamples.at(i)>max)
                max = data.recordedSamples.at(i);
        }
        std::cout<< "Max amp of " << max << std::endl;
    } else{
        std::cout<<errorstr<<std::endl;
    }
    return 0;
}
