/*
  ==============================================================================

    MoogCat.cpp
    Created: 16 Oct 2020 12:02:43pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#include "MoogCat.h"

//constructor
MoogCat::MoogCat(AudioProcessorValueTreeState& vts): state(2){

    cutoffParam = vts.getRawParameterValue(FREQUENCY_ID);
    resParam = vts.getRawParameterValue(RESONANCE_ID);
    morphParam = vts.getRawParameterValue(MORPHING_ID);

    cutoffSmoother.reset(NUM_STEPS);
    resonanceSmoother.reset(NUM_STEPS);
    morphingSmoother.reset(NUM_STEPS);

    fc = *cutoffParam;
    res = *resParam;
    r_cat = *morphParam;
    g = dsp::FastMathApproximations::tan(juce::MathConstants<float>::pi * fc / Fs);

    setSampleRate(44100.0f);
    setCutoffFrequency(*cutoffParam);
    setResonance(*resParam);
    setFilterType(*morphParam);
}

//destructor
MoogCat::~MoogCat(){
    
}

void MoogCat::createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>>& params) {

    params.push_back(std::make_unique<AudioParameterFloat>(FREQUENCY_ID, FREQUENCY_NAME, NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.1989f), 800.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(RESONANCE_ID, RESONANCE_NAME, NormalisableRange<float>(0.0f, 1.0f, 0.001f, 0.5f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(MORPHING_ID, MORPHING_NAME, NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.564f));

}

//cutoff getter and setter
void MoogCat::setCutoffFrequency(float cutoff_frequency) noexcept{
    cutoffSmoother.setTargetValue(cutoff_frequency);
}

//resonance getter and setter
void MoogCat::setResonance(float resn) noexcept {
    resonanceSmoother.setTargetValue(resonanceMap(resn));
}

// coeeficient of filter type getter and setter
void MoogCat::setFilterType(float r) noexcept {
    morphingSmoother.setTargetValue(filterTypeMap(r));
}

//sampleRate getter and setter
void MoogCat::setSampleRate(float sampleRate) noexcept {
    Fs = sampleRate;
}


//prepare the filter
void MoogCat::prepare(const juce::dsp::ProcessSpec& spec){
    
    setSampleRate((float) spec.sampleRate);
    setNumChannels(spec.numChannels);

    cutoffSmoother.skip(NUM_STEPS);
    morphingSmoother.skip(NUM_STEPS);
    resonanceSmoother.skip(NUM_STEPS);

    reset();
}

//reset the filter
void MoogCat::reset(){
    
    for (auto& s:state) {
        s.fill(0.0);
    }

}

//filter processing method
//input to be processed
//channel to be processed
//TODO: reduce complexity of this process
float MoogCat::processSample(float input, size_t channel) noexcept {
    auto& s = state[channel];
    const auto den = (4*res*g*g*g*g*r_cat*r_cat + g*g*g*g + 4*g*g*g*r_cat + 4*g*g*r_cat*r_cat + 2*g*g + 4*g*r_cat + 1);
    float out = (g*g*g*s[0] - g*g*(2*g*r_cat + 1)*s[1] + g*(g*g + 2*g*r_cat + 1)*s[2] - (2*g*g*g*r_cat + 4*g*g*r_cat*r_cat + g*g + 4*g*r_cat + 1)*s[3])/den;
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

void MoogCat::updateSmoothers() noexcept {
    
    const float nextFreq = cutoffSmoother.getNextValue();
    const float nextResonance = resonanceSmoother.getNextValue();
    const float nextMorphing = morphingSmoother.getNextValue();
    
    fc = nextFreq;
    g = dsp::FastMathApproximations::tan ( juce::MathConstants<float>::pi * fc / Fs );
    res = nextResonance;
    r_cat = nextMorphing;
}

void MoogCat::process(AudioBuffer<float>& buffer){

    setCutoffFrequency(*cutoffParam);
    setResonance(*resParam);
    setFilterType(*morphParam);


    for (size_t n = 0; n < buffer.getNumSamples(); ++n){

        updateSmoothers();

        for (size_t ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.getWritePointer(ch)[n] = processSample (buffer.getReadPointer (ch)[n], ch);
    }
}