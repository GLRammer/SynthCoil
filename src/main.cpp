#include "audio.h"
#include "freqs.h"
#include "main.h"
#include <string>
#include <iostream>

int main()
{
    paData data;
    std::string errorstr = "";
    if (spinUp(5.0, data, errorstr) == 0)
    {
        SAMPLE max = 0;
        for (int i = 0; i < (int)data.recordedSamples.size(); i++)
        {
            if (data.recordedSamples[i] > max)
                max = data.recordedSamples[i];
        }

        // freq sample
        freqHolder tempout = freqGet(data.recordedSamples);
        std::vector<SAMPLE> freqs = tempout.frequencies, mags = tempout.magnitudes;
        for (int i = FFTSZ; i + FFTSZ < data.recordedSamples.size(); i += FFTSZ)
        {
            tempout = freqGet(data.recordedSamples, i);
            freqs.insert(freqs.end(), tempout.frequencies.begin(), tempout.frequencies.end());
            mags.insert(mags.end(), tempout.magnitudes.begin(), tempout.magnitudes.end());
        }
        // analyze sample
        max = 0;
        SAMPLE maxFreq;
        for (int i = 0; i < freqs.size(); i++)
        {
            if (mags[i] > max)
            {
                max = mags[i];
                maxFreq = freqs[i];
            }
        }
        std::cout << "Max freq= " << maxFreq << std::endl;
    }
    else
    {
        std::cout << errorstr << std::endl;
    }
    return 0;
}
