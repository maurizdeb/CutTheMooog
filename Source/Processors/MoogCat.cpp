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
void MoogCat::reset() {

    for (auto &s:state) {
        s.fill(0.0f);
    }
}

//filter processing method
//input to be processed
//channel to be processed
//TODO: reduce complexity of this process
float MoogCat::processSample(float input, size_t channel) noexcept {
    auto& s = state[channel];
    const float g_2 = g*g;
    const float g_3 = g_2*g;
    const float g_4 = g_3*g;
    const float r_cat_2 = r_cat*r_cat;
    const float val1 = res*g_4*r_cat_2;
    const float val2 = g_3*r_cat;
    const float val3 = g*r_cat;
    const float val4 = g_2*r_cat_2;
    const float val5 = 2*val3 + 1;
    const float val6 = g_2 + val5;
    const float val7 = g*val6;
    const auto den = (4*val1 + g_4 + 4*val2 + 4*val4 + 2*g_2 + 4*val3 + 1);
    float out = (g_3*s[0] - g_2*(val5)*s[1] + val7*s[2] - (2*val2 + 4*val4 + g_2 + 4*val3 + 1)*s[3])/den;
    const float a = -(val1*4 + g_4 + 4*val2 + 4*val4 - 1)*s[0] + 2*g*(res*val4*4 + val6)*s[1] - 8*g_2*res*r_cat_2*s[2] + 8*g*res*r_cat_2*(2*val3+1)*s[3] + val7*2*input;
    const float b = - 2*val7*s[0] + (-val1*4 - g_4 + 4*val4 + 4*val3 + 1)*s[1] + 8*g_3*res*r_cat_2*s[2] - 8*g_2*res*r_cat_2*(2*val3+1)*s[3] - g_2*(val6)*2*input;
    const float c = 2*g_2*s[0] - g*(val5)*2*s[1] - (val1*4 + g_4 + 4*g_3*r_cat + 4*val4 - 1)*s[2] + 2*g*(res*val4*4 + val6)*s[3] + 2*g_3*input;
    const float d = -2*g_3*s[0] + g_2*(val5)*2*s[1] - val7*2*s[2] + (-val1*4 - g_4 + 4*val4 + 4*val3 + 1)*s[3] - 2*g_4*input;
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