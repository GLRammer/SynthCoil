#include "freqs.h"

freqHolder::freqHolder(std::vector<float> freqs, std::vector<float> mags)
{
    // frequencies.swap(freqs);
    // magnitudes.swap(mags);
    frequencies = freqs;
    magnitudes = mags;
}

freqHolder freqGet(std::vector<float> in, int loc)
{
    // Expected size of output vector
    int outSz = (FFTSZ / 2) + 1;

    // Output vector
    std::vector<fftwf_complex> out(outSz);

    // set up subvector
    std::vector<float> subIn(FFTSZ);

    // Setup fftw and execute
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(FFTSZ, subIn.data(), out.data(), FFTW_MEASURE);
    memcpy(subIn.data(), in.data(), FFTSZ);
    fftwf_execute(plan);
    // Vector for holding magnitudes
    std::vector<float> mags(outSz);

    // Calculation of magnitudes
    for (int i = 0; i < outSz; i++)
    {
        // real num
        float re = out[i][0];
        // imaginary num
        float im = out[i][1];
        // actual calculations
        float mag = std::hypot(re, im);
        mags.push_back(mag / (float)outSz);
    }

    // Vector for holding frequencies
    std::vector<float> freqs(outSz);

    // Calculation of frequencies
    for (int i = 0; i < outSz; i++)
    {
        freqs.push_back((float)i * (float)FFTSZ / (float)in.size());
    }
    
    // Return frequencies and their magnitudes
    return freqHolder(freqs, mags);
}