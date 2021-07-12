/*
  ==============================================================================

    MoogCat.h
    Created: 16 Oct 2020 12:02:43pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#define FREQUENCY_ID "frequency"
#define FREQUENCY_NAME "Frequency"
#define RESONANCE_ID "resonance"
#define RESONANCE_NAME "Resonance"
#define MORPHING_ID "morphing"
#define MORPHING_NAME "Morphing"

#define NUM_STEPS 500

class MoogCat{

public:
    
    //constructor and destructor methods
    MoogCat(AudioProcessorValueTreeState& vts);
    ~MoogCat();
    
    //cutoff getter and setter
    void setCutoffFrequency(float cutoff_frequency) noexcept;
    
    //resonance getter and setter
    void setResonance(float resonance) noexcept;
    
    
    // coefficient of filter type getter and setter
    void setFilterType(float r) noexcept;
    
    
    //sampleRate getter and setter
    void setSampleRate(float sampleRate) noexcept;
    
    size_t getNumChannels() const noexcept       { return state.size(); }
    
    //prepare the filter
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    //reset the filter
    void reset();

    void process(AudioBuffer<float>& buffer);

    static void createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>>& params);
    
protected:
    //filter processing method
    //input to be processed
    //channel to be processed
    float processSample(float input, size_t channel) noexcept;

private:

    // filter params
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resParam = nullptr;
    std::atomic<float>* morphParam = nullptr;

    //Filter CURRENT SMOOTHED values
    float fc = 800.0f; //cutoff frequency
    float res = 0.0f; //resonance
    float Fs = 44100.0f; //sampling frequency
    float g = dsp::FastMathApproximations::tan(juce::MathConstants<float>::pi * fc / Fs); //frequency warping factor
    float r_cat = 1.064f; //filter type value

    //Smoothers
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> cutoffSmoother;
    SmoothedValue<float, ValueSmoothingTypes::Linear> resonanceSmoother, morphingSmoother;


    //vector of states
    static constexpr size_t numStates = 4;
    std::vector<std::array<float, numStates>> state;

    void setNumChannels (size_t newValue)   { state.resize (newValue); }

    //State-space matrices initialization
    void updateSmoothers() noexcept;

    float resonanceMap(float resn) noexcept {return jlimit<float>(0.0f, 0.99f, resn);}
    float filterTypeMap(float map) noexcept {return jmap<float>(map, 0.4f, 1.5f); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoogCat)
};
