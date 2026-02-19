#include "freqs.h"

freqHolder::freqHolder(std::vector<double> freqs,std::vector<double> mags){
    // frequencies.swap(freqs);
    // magnitudes.swap(mags);
    frequencies=freqs;
    magnitudes=mags;
}

freqHolder freqGet(std::vector<double> in)
{
    int tempsz = FFTSZ / 2 + 1;
    std::vector<fftw_complex> out(tempsz);
    fftw_plan plan = fftw_plan_dft_r2c_1d(FFTSZ, in.data(), reinterpret_cast<fftw_complex *>(out.data()), FFTW_MEASURE);
    fftw_execute(plan);
    std::vector<double> mags(tempsz);
    for (int i = 0; i < tempsz; i++)
    {
        double re = out[i][0];
        double im = out[i][1];
        double mag = std::hypot(re, im);
        mags[i] = mag / tempsz;
    }
    std::vector<double> freqs(tempsz);
    for (int i = 0; i < tempsz; i++)
    {
        freqs[i]=(double) i * (double) FFTSZ / (double)in.size();
    }
    return freqHolder(freqs,mags);
}