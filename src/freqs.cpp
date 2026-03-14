#include "freqs.h"

freqHolder::freqHolder(){
    // Expected size of output vector
    int outSz = (FFTSZ / 2) + 1;

    // Output vector and input buffer
    out=(fftwf_complex*) malloc(sizeof(fftwf_complex)*outSz);
    freqbuff=std::vector<float>();
    freqbuff.reserve(FFTSZ);

    // Setup fftw
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(FFTSZ, freqbuff.data(), out, FFTW_MEASURE);
}


freqHolder::~freqHolder(){
    fftwf_destroy_plan(plan);
    free(out);
}

void freqHolder::freqGet(audio in)
{
    // Expected size of output vector
    int outSz = (FFTSZ / 2) + 1;
    float tempbuff[FFTSZ];
    if(in.catchStream(tempbuff,sizeof(tempbuff))==-1)
        return;
    freqbuff.clear();
    freqbuff.assign(tempbuff,&tempbuff[FFTSZ]);
    fftwf_execute(plan);

    // clear magnitude vector
    magnitudes.clear();
    magnitudes.reserve(outSz);

    // Calculation of magnitudes
    for (int i = 0; i < outSz; i++)
    {
        // real num
        float re = out[i][0];
        // imaginary num
        float im = out[i][1];
        // actual calculations
        float mag = std::hypot(re, im);
        magnitudes.push_back(mag / (float)outSz);
    }

    //  Clear frequency vector
    frequencies.clear();
    frequencies.reserve(outSz);

    // Calculation of frequencies
    for (int i = 0; i < outSz; i++)
    {
        frequencies.push_back((float)i * (float)SAMPLE_RATE / (float)FFTSZ);
    }
    
}