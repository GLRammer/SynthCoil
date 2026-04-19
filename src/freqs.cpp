#include "freqs.h"

freqHolder::freqHolder()
{
    // Output vector and input buffer
    out.resize(OUTSZ);
    freqbuff.resize(FFTSZ);

    // Setup fftw
    plan = fftwf_plan_dft_r2c_1d(FFTSZ, freqbuff.data(), reinterpret_cast<fftwf_complex*>(out.data()), FFTW_MEASURE);
    frequencies.clear();
    magnitudes.clear();

}

freqHolder::~freqHolder()
{
    fftwf_destroy_plan(plan);
}

void freqHolder::freqGet(audio &in)
{
    if (in.catchStream((char *)freqbuff.data(), sizeof(float) * FFTSZ) == -1)
        return;

    fftwf_execute(plan);

    // clear magnitude vector
    magnitudes.clear();
    magnitudes.reserve(OUTSZ);
    topN=std::vector<std::pair<float,float>>(topNum,std::pair<float,float>(0.0,0.0));

    // store multiple frames of output for smoother animation
    while(outFrames.size()>=tweenFrames){
        outFrames.pop_back();
    }
    outFrames.push_front(out);

    // store avg frame for tweening
    std::vector<std::complex<float>> avgOut;
    avgOut.resize(OUTSZ);
    if(tweenFrames>1){
        for(const std::vector<std::complex<float>>& frame : outFrames){
            for(int i=0;i<OUTSZ;i++){
                avgOut[i]+=frame[i];
            }
        }
        for(int i=0;i<OUTSZ;i++){
            avgOut[i]/=outFrames.size();
        }
    }else{
        for(int i=0;i<OUTSZ;i++){
            avgOut[i]=out[i];
        }
    }

    // store multiple frames of peaks for smooth automatic gain control
    while(peak.size()>=tweenFrames){
        peak.pop_back();
    }
    peak.push_front({0.0f, 0.0f});

    // Change frequency range to indexes, min cannot go below 1 to account for human hearing range, max range handled elsewhere
    int minInd=(int)((min*OUTSZ*2.0)/(float)SAMPLE_RATE);
    int maxInd=(int)((max*OUTSZ*2.0)/(float)SAMPLE_RATE);
    if(minInd<=0){
        minInd=1;
    }
    // Shouldn't be necessary in theory, but here as a safety precaution
    if(maxInd>=OUTSZ){
        maxInd=OUTSZ-1;
    }

    // Calculation of magnitudes
    for (int i = minInd; i <= maxInd; i++)
    {
        // actual calculations
        float mag = std::abs(avgOut[i]) / FFTSZ;
        if(i!=OUTSZ)
            mag*=2;
        mag/=gain;
        magnitudes.push_back(mag);
        for(int j=0;j<topNum;j++){
            if(mag>topN[j].second){
                topN[j].first=i;
                topN[j].second=mag;
                break;
            }
        }
        if (peak.front().first < mag)
        {
            peak.front().first = mag;
            peak.front().second = i;
        }
    }
    if(autoGain){
        gain=0;
        for(auto&& val : peak){
            gain+=val.first;
        }
        gain/=peak.size();
    }

    //  Clear frequency vector
    frequencies.clear();
    frequencies.reserve(OUTSZ);

    // Calculation of frequencies
    for (int i = minInd; i <= maxInd; i++)
    {
        float freq = (float)i * (float)SAMPLE_RATE / (OUTSZ*2.0);
        frequencies.push_back(freq);
        if (i == peak.front().second)
            peak.front().second = freq;
    }
    for(int i=0;i<topNum;i++){
        topN[i].first=melConv(frequencies[topN[i].first],frequencies[minInd],frequencies[maxInd-1]);
        // topN[i].first=frequencies[topN[i].first]/OUTSZ;
    }
}

const std::vector<float> freqHolder::getFrequencies(){return frequencies;}
const std::vector<float> freqHolder::getMagnitudes(){return magnitudes;}
const std::pair<float, float> freqHolder::getPeak() { return peak.front(); }
const std::vector<std::pair<float, float>> freqHolder::getTop(){return topN;}

void freqHolder::setMinMax(const float newmin, const float newmax){
    // Clamp values to upper range of human hearing
    if(newmin>=HEARINGMAX){
        min=HEARINGMAX-1.0;
    }else if(newmin<=20){
        min=20;
    }else{
        min=newmin;
    }
    if(newmax>=HEARINGMAX){
        max=HEARINGMAX;
    }else if(newmax<=min){
        max=min+(SAMPLE_RATE/(OUTSZ*2.0));
    }else{
        max=newmax;
    }
}

void freqHolder::setGain(const float newgain){
    if(newgain==-1.0){
        autoGain=true;
        return;
    }
    autoGain=false;
    if(newgain<=0.0){
        gain=0.0;
    }else if(newgain>=1.0){
        gain=1.0;
    }
}

void freqHolder::setFrames(const int newframes){
    if(newframes>0){
        tweenFrames=newframes;
    }else{
        tweenFrames=1;
    }
}

void freqHolder::setTop(const int n){
    if(n<=0){
        topNum=1;
    }else{
        topNum=n;
    }
}

float melConv(float input, float min, float max){
    if(input<=0.0){
        return 0.0;
    }
    float melMin = melHelp(min);
    float melMax = melHelp(max);
    float melIn = melHelp(input);
    return (melIn-melMin)/(melMax-melMin);
}

float melHelp(float freq){
    return 2595.0 * log10f(1.0+freq/700.0);
}