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
    g = dsp::FastMathApproximations::tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
}

template <typename SampleType>
//destructor
MoogCat<SampleType>::~MoogCat(){
    
}
template <typename SampleType>
//cutoff getter and setter
void MoogCat<SampleType>::setCutoffFrequency(SampleType cutoff_frequency) noexcept{
    fc = cutoff_frequency;
}

template <typename SampleType>
//resonance getter and setter
void MoogCat<SampleType>::setResonance(SampleType resonance) noexcept {
    res = resonanceMap(resonance);
}

template <typename SampleType>
// coeeficient of filter type getter and setter
void MoogCat<SampleType>::setFilterType(SampleType r) noexcept {
    r_cat = filterTypeMap(r);
}
template <typename SampleType>
//sampleRate getter and setter
void MoogCat<SampleType>::setSampleRate(SampleType sampleRate) noexcept {
    Fs = sampleRate;
}
template <typename SampleType>
//update the filter
void MoogCat<SampleType>::initFilter() noexcept{
    
    g = dsp::FastMathApproximations::tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
}

template <typename SampleType>
void MoogCat<SampleType>::initFilter(SampleType cutoff, SampleType resonance, SampleType type) noexcept {
    
    fc = cutoff;
    res = resonanceMap(resonance);
    r_cat = filterTypeMap(type);
    cutoffSmoother.setCurrentAndTargetValue(fc);
    resonanceSmoother.setCurrentAndTargetValue(res);
    morphingSmoother.setCurrentAndTargetValue(r_cat);
    
    g = dsp::FastMathApproximations::tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
}

template <typename SampleType>
void MoogCat<SampleType>::update(SampleType cutoff, SampleType resonance, SampleType type) noexcept {
    
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
SampleType MoogCat<SampleType>::processSample(SampleType input, size_t channel) noexcept {
    auto& s = state[channel];
    const auto den = (4*res*g*g*g*g*r_cat*r_cat + g*g*g*g + 4*g*g*g*r_cat + 4*g*g*r_cat*r_cat + 2*g*g + 4*g*r_cat + 1);
    SampleType out = (g*g*g*s[0] - g*g*(2*g*r_cat + 1)*s[1] + g*(g*g + 2*g*r_cat + 1)*s[2] - (2*g*g*g*r_cat + 4*g*g*r_cat*r_cat + g*g + 4*g*r_cat + 1)*s[3])/den;
    const auto a = -(res*g*g*g*g*r_cat*r_cat*4 + g*g*g*g + 4*g*g*g*r_cat + 4*g*g*r_cat*r_cat - 1)*s[0] + 2*g*(res*g*g*r_cat*r_cat*4 + g*g + 2*g*r_cat + 1)*s[1] - 8*g*g*res*r_cat*r_cat*s[2] + 8*g*res*r_cat*r_cat*(2*g*r_cat+1)*s[3] + g*(g*g + r_cat*2*g + 1)*2*input;
    const auto b = - 2*g*(g*g + r_cat*2*g + 1)*s[0] + (-res*g*g*g*g*r_cat*r_cat*4 - g*g*g*g + 4*g*g*r_cat*r_cat + 4*g*r_cat + 1)*s[1] + 8*g*g*g*res*r_cat*r_cat*s[2] - 8*g*g*res*r_cat*r_cat*(2*g*r_cat+1)*s[3] - g*g*(g*g + r_cat*2*g + 1)*2*input;
    const auto c = 2*g*g*s[0] - g*(2*g*r_cat + 1)*2*s[1] - (res*g*g*g*g*r_cat*r_cat*4 + g*g*g*g + 4*g*g*g*r_cat + 4*g*g*r_cat*r_cat - 1)*s[2] + 2*g*(res*g*g*r_cat*r_cat*4 + g*g + 2*g*r_cat + 1)*s[3] + 2*g*g*g*input;
    const auto d = -2*g*g*g*s[0] + g*g*(2*g*r_cat + 1)*2*s[1] - g*(g*g + r_cat*2*g + 1)*2*s[2] + (-res*g*g*g*g*r_cat*r_cat*4 - g*g*g*g + 4*g*g*r_cat*r_cat + 4*g*r_cat + 1)*s[3] - 2*g*g*g*g*input;
    s[0] = a/den;
    s[1] = b/den;
    s[2] = c/den;
    s[3] = d/den;
    return out;
}

template <typename SampleType>
void MoogCat<SampleType>::updateSmoothersAndStateSpace() noexcept {
    
    const auto nextFreq = cutoffSmoother.getNextValue();
    const auto nextResonance = resonanceSmoother.getNextValue();
    const auto nextMorphing = morphingSmoother.getNextValue();
    
    fc = nextFreq;
    g = dsp::FastMathApproximations::tan(SampleType(juce::MathConstants<float>::pi)*fc/Fs);
    res = resonanceMap(nextResonance);
    r_cat = filterTypeMap(nextMorphing);
}

template class MoogCat<float>;
template class MoogCat<double>;
