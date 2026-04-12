#include "freqs.h"

freqHolder::freqHolder()
{
    // Expected size of output vector
    int outSz = (FFTSZ / 2) + 1;

    // Output vector and input buffer
    out = (fftwf_complex *)malloc(sizeof(fftwf_complex) * outSz);
    freqbuff = (float *)malloc(sizeof(float) * FFTSZ);

    // Setup fftw
    plan = fftwf_plan_dft_r2c_1d(FFTSZ, freqbuff, out, FFTW_MEASURE);
    frequencies.clear();
    magnitudes.clear();
}

freqHolder::~freqHolder()
{
    fftwf_destroy_plan(plan);
    free(out);
    free(freqbuff);
}

void freqHolder::freqGet(audio &in)
{
    // Expected size of output vector
    int outSz = (FFTSZ / 2) + 1;

    if (in.catchStream((char *)freqbuff, sizeof(float) * FFTSZ) == -1)
        return;

    fftwf_execute(plan);

    // clear magnitude vector
    magnitudes.clear();
    magnitudes.reserve(outSz);
    topN=std::vector<std::pair<float,float>>(topNum,std::pair<float,float>(0.0,0.0));
    peak = {0.0f, 0.0f};

    // Calculation of magnitudes
    for (int i = 0; i < outSz; i++)
    {
        // real num
        float re = out[i][0];
        // imaginary num
        float im = out[i][1];
        // actual calculations
        float mag = std::hypot(re, im) / (float)outSz;
        magnitudes.push_back(mag);
        for(int j=0;j<topNum;j++){
            if(mag>topN[j].second){
                topN[j].first=i;
                topN[j].second=mag;
                break;
            }
        }
        if (peak.first < mag)
        {
            peak.first = mag;
            peak.second = i;
        }
    }

    //  Clear frequency vector
    frequencies.clear();
    frequencies.reserve(outSz);

    // Calculation of frequencies
    for (int i = 0; i < outSz; i++)
    {
        float freq = (float)i * (float)SAMPLE_RATE / (float)FFTSZ;
        frequencies.push_back(freq);
        if (i == peak.second)
            peak.second = freq;
    }
    for(int i=0;i<topNum;i++){
        topN[i].first=frequencies[topN[i].first]/(float)outSz;
    }
}

std::vector<float> freqHolder::getFrequencies(){return frequencies;}
std::vector<float> freqHolder::getMagnitudes(){return magnitudes;}
std::pair<float, float> freqHolder::getPeak() { return peak; }
std::vector<std::pair<float, float>> freqHolder::getTop(){return topN;}
void freqHolder::setTop(int n){topNum=n;}