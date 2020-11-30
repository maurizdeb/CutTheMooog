/*
  ==============================================================================

    MoogCat.cpp
    Created: 16 Oct 2020 12:02:43pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#include "MoogCat.h"

template <typename SampleType>
//constructor
MoogCat<SampleType>::MoogCat(): state(2){
    
    cutoffSmoother.setCurrentAndTargetValue(cutoffSmoother.getTargetValue());
    resonanceSmoother.setCurrentAndTargetValue(resonanceSmoother.getTargetValue());
    morphingSmoother.setCurrentAndTargetValue(morphingSmoother.getTargetValue());
    
    fc = SampleType(800);
    res = SampleType(0);
    r_cat = SampleType(1.064);
    g = tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
}

template <typename SampleType>
//destructor
MoogCat<SampleType>::~MoogCat(){
    
}
template <typename SampleType>
//cutoff getter and setter
void MoogCat<SampleType>::setCutoffFrequency(SampleType cutoff_frequency){
    fc = cutoff_frequency;
}

template <typename SampleType>
//resonance getter and setter
void MoogCat<SampleType>::setResonance(SampleType resonance){
    res = resonanceMap(resonance);
}

template <typename SampleType>
// coeeficient of filter type getter and setter
void MoogCat<SampleType>::setFilterType(SampleType r){
    r_cat = filterTypeMap(r);
}
template <typename SampleType>
//sampleRate getter and setter
void MoogCat<SampleType>::setSampleRate(SampleType sampleRate){
    Fs = sampleRate;
}
template <typename SampleType>
//update the filter
void MoogCat<SampleType>::initFilter(){
    
    g = tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
    setFilterMatrices();
}

template <typename SampleType>
void MoogCat<SampleType>::initFilter(SampleType cutoff, SampleType resonance, SampleType type){
    
    fc = cutoff;
    res = resonanceMap(resonance);
    r_cat = filterTypeMap(type);
    cutoffSmoother.setCurrentAndTargetValue(fc);
    resonanceSmoother.setCurrentAndTargetValue(res);
    morphingSmoother.setCurrentAndTargetValue(r_cat);
//    setCutoffFrequency(cutoff);
//    setResonance(resonance);
//    setFilterType(type);
    
    g = tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
    setFilterMatrices();
}

template <typename SampleType>
void MoogCat<SampleType>::update(SampleType cutoff, SampleType resonance, SampleType type){
    
    cutoffSmoother.setTargetValue(cutoff);
    resonanceSmoother.setTargetValue(resonanceMap(resonance));
    morphingSmoother.setTargetValue(filterTypeMap(type));
    
}

template <typename SampleType>
//prepare the filter
void MoogCat<SampleType>::prepare(const juce::dsp::ProcessSpec& spec){
    
    setSampleRate(SampleType(spec.sampleRate));
    setNumChannels(spec.numChannels);
    cutoffSmoother.reset(spec.sampleRate, SampleType(0.05));
    resonanceSmoother.reset(spec.sampleRate, SampleType(0.05));
    morphingSmoother.reset(spec.sampleRate, SampleType(0.05));
    
    initFilter();
    reset();
}

template <typename SampleType>
//reset the filter
void MoogCat<SampleType>::reset(){
    
    for (auto& s:state) {
        s.fill(SampleType(0));
    }
}

template <typename SampleType>
//filter processing method
//input to be processed
//channel to be processed
SampleType MoogCat<SampleType>::processSample(SampleType input, size_t channel){
    auto& s = state[channel];
    SampleType out = C[0]*s[0] + C[1]*s[1] + C[2]*s[2] + C[3]*s[3];
    const auto a = A[0][0]*s[0] + A[0][1]*s[1] + A[0][2]*s[2] + A[0][3]*s[3] + B[0]*input;
    const auto b = A[1][0]*s[0] + A[1][1]*s[1] + A[1][2]*s[2] + A[1][3]*s[3] + B[1]*input;
    const auto c = A[2][0]*s[0] + A[2][1]*s[1] + A[2][2]*s[2] + A[2][3]*s[3] + B[2]*input;
    const auto d = A[3][0]*s[0] + A[3][1]*s[1] + A[3][2]*s[2] + A[3][3]*s[3] + B[3]*input;
    s[0] = a;
    s[1] = b;
    s[2] = c;
    s[3] = d;
    return out;
}

template <typename SampleType>
//This function initialize the State-Space matrix A based on the MATLAB matrix
void MoogCat<SampleType>::init_A(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2){
    
    
    const auto val1 = SampleType(2)*g*r_cat + SampleType(1);
    const auto val7 = res*pow_g_4*pow_r_cat_2*SampleType(4);
    const auto val8 = SampleType(4)*pow_g_2*pow_r_cat_2;
    const auto val9 = SampleType(4)*pow_g_3*r_cat;
    const auto val6 = val7 + pow_g_4 + val9 + val8 + SampleType(2)*pow_g_2 + SampleType(4)*g*r_cat+SampleType(1);
    const auto val2 = - (val7 + pow_g_4 + val9 +val8 -SampleType(1))/val6;
    const auto val3 = (-val7 - pow_g_4 + val8 + SampleType(4)*g*r_cat+SampleType(1))/val6;
    const auto val4 = SampleType(2)*g*(res*pow_g_2*pow_r_cat_2*SampleType(4) + pow_g_2 + SampleType(2)*g*r_cat +SampleType(1))/val6;
    const auto val5 = -(SampleType(2)*g*(pow_g_2 + r_cat*g*SampleType(2) +SampleType(1)))/val6;
    
    A[0][0] = val2;
    A[0][1] = val4;
    A[0][2] = - (SampleType(8)*pow_g_2*pow_r_cat_2*res)/val6;
    A[0][3] = (g*pow_r_cat_2*res*val1*SampleType(8))/val6;
    A[1][0] = val5;
    A[1][1] = val3;
    A[1][2] = (SampleType(8)*pow_g_3*pow_r_cat_2*res)/val6;
    A[1][3] = -(pow_g_2*pow_r_cat_2*res*val1*SampleType(8))/val6;
    A[2][0] = SampleType(2)*pow_g_2/val6;
    A[2][1] = -SampleType(2)*g*val1/val6;
    A[2][2] = val2;
    A[2][3] = val4;
    A[3][0] = -SampleType(2)*pow_g_3/val6;
    A[3][1] = SampleType(2)*pow_g_2*val1/val6;
    A[3][2] = val5;
    A[3][3] = val3;
}

template <typename SampleType>
//This function initialize the State-Space matrix B based on the MATLAB matrix
void MoogCat<SampleType>::init_B(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2){
    
    
    const auto val1 = res*pow_g_4*pow_r_cat_2*SampleType(4) + pow_g_4 + SampleType(4)*pow_g_3*r_cat + SampleType(4)*pow_g_2*pow_r_cat_2 + SampleType(2)*pow_g_2 + SampleType(4)*g*r_cat +SampleType(1);
    
    B[0] = SampleType(2)*g*(pow_g_2 + r_cat*g*SampleType(2) +SampleType(1))/val1;
    B[1] = -SampleType(2)*g*(pow_g_3 + r_cat*pow_g_2*SampleType(2) + g)/val1;
    B[2] = SampleType(2)*pow_g_3/val1;
    B[3] = - SampleType(2)*pow_g_4/val1;
}

template <typename SampleType>
//This function initialize the State-Space matrix C based on the MATLAB matrix
void MoogCat<SampleType>::init_C(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2){
    
    const auto val2 = res*pow_g_4*pow_r_cat_2*SampleType(4) + pow_g_4 + SampleType(4)*pow_g_3*r_cat + SampleType(4)*pow_g_2*pow_r_cat_2 + SampleType(2)*pow_g_2 + SampleType(4)*g*r_cat + SampleType(1);
    const auto val3 = pow_g_2 + r_cat*g*SampleType(2) + SampleType(1);
    const auto val1 = pow_g_2*val3/val2;
    
    C[0] = (pow_g_3*(SampleType(2)*g*r_cat+SampleType(1)) - SampleType(2)*pow_g_4*r_cat)/val2;
    C[1] = (pow_g_4/val2 - val1);
    C[2] = (g*(SampleType(2)*g*r_cat + SampleType(1))*val3 - (pow_g_2*r_cat*val3*SampleType(2)))/val2;
    C[3] = val1 + ((SampleType(4)*pow_g_4*pow_r_cat_2*res)/(val2)) -SampleType(1);
}

template <typename SampleType>
void MoogCat<SampleType>::setFilterMatrices(){
    
    const auto pow_g_2 = pow(g,SampleType(2));
    const auto pow_g_3 = pow(g,SampleType(3));
    const auto pow_g_4 = pow(g,SampleType(4));
    const auto pow_r_cat_2 = pow(r_cat,SampleType(2));
    
    init_A(pow_g_2, pow_g_3, pow_g_4,pow_r_cat_2);
    init_B(pow_g_2, pow_g_3, pow_g_4, pow_r_cat_2);
    init_C(pow_g_2, pow_g_3, pow_g_4, pow_r_cat_2);
}

template <typename SampleType>
void MoogCat<SampleType>::updateSmoothersAndStateSpace(){
    
//    const auto currentFreq = cutoffSmoother.getCurrentValue();
//    const auto currentRes = resonanceSmoother.getCurrentValue();
//    const auto currentMorph = morphingSmoother.getCurrentValue();
    
    const auto nextFreq = cutoffSmoother.getNextValue();
    const auto nextResonance = resonanceSmoother.getNextValue();
    const auto nextMorphing = morphingSmoother.getNextValue();
    
    if (nextFreq != fc || nextResonance != resonanceMap(res) || nextMorphing != filterTypeMap(r_cat)) {
        if (nextFreq != fc){
            fc = nextFreq;
            g = tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
        }
        if (nextResonance != resonanceMap(res))
            res = resonanceMap(nextResonance);
        if (nextMorphing != filterTypeMap(r_cat))
            r_cat = filterTypeMap(nextMorphing);
        setFilterMatrices();
    }else{
//        cutoffSmoother.setCurrentAndTargetValue(fc);
//        resonanceSmoother.setCurrentAndTargetValue(resonanceMap(res));
//        morphingSmoother.setCurrentAndTargetValue(res);
    }
    
}

template class MoogCat<float>;
template class MoogCat<double>;
