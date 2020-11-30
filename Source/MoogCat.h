/*
  ==============================================================================

    MoogCat.h
    Created: 16 Oct 2020 12:02:43pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

template <typename SampleType>
class MoogCat{
    
private:
    
    //Filter CURRENT values
    SampleType fc; //cutoff frequency
    SampleType res; //resonance
    SampleType g; //frequency warping factor
    SampleType Fs; //sampling frequency
    SampleType r_cat; //filter type value
    
    //Smoothers
    SmoothedValue<SampleType, ValueSmoothingTypes::Multiplicative> cutoffSmoother;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> resonanceSmoother, morphingSmoother;
    
    //State-Space matrices
    SampleType A[4][4];// = {0}
    SampleType B[4];// = {0};
    SampleType C[4];// = {0};
    
    //vector of states
    static constexpr size_t numStates = 4;
    std::vector<std::array<SampleType, numStates>> state;
    
    void setNumChannels (size_t newValue)   { state.resize (newValue); }
    
    //State-space matrices initialization
    void updateSmoothersAndStateSpace();
    void init_A(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2);
    void init_B(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2);
    void init_C(SampleType pow_g_2, SampleType pow_g_3, SampleType pow_g_4, SampleType pow_r_cat_2);
    void setFilterMatrices();
    
    SampleType resonanceMap(SampleType resonance) {return jmap(resonance, SampleType(0), SampleType(0.99));}
    SampleType filterTypeMap(SampleType map) {return jmap(map, SampleType(0.4), SampleType(1.5)); }
    
public:
    
    //constructor and destructor methods
    MoogCat();
    ~MoogCat();
    
    //cutoff getter and setter
    void setCutoffFrequency(SampleType cutoff_frequency);
    
    //resonance getter and setter
    void setResonance(SampleType resonance);
    
    
    // coeeficient of filter type getter and setter
    void setFilterType(SampleType r);
    
    
    //sampleRate getter and setter
    void setSampleRate(SampleType sampleRate);
    
    size_t getNumChannels() const noexcept       { return state.size(); }
    
    //initialize the filter
    void initFilter();
    void initFilter(SampleType cutoff, SampleType resonance, SampleType type);
    
    //update the filter
    void update(SampleType cutoff, SampleType resonance, SampleType type);
    
    //prepare the filter
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    //reset the filter
    void reset();
    
    template <typename ProcessContext>
    void process(const ProcessContext& context){
        
        const auto& inputBlock = context.getInputBlock();
        auto outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
        
        jassert (inputBlock.getNumChannels() <= getNumChannels());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);
        
        if (context.isBypassed) {
            outputBlock.copyFrom (inputBlock);
            return;
        }
        for (size_t n = 0; n < numSamples; ++n){
            
            updateSmoothersAndStateSpace();
                    
            for (size_t ch = 0; ch < numChannels; ++ch)
                outputBlock.getChannelPointer (ch)[n] = processSample (inputBlock.getChannelPointer (ch)[n], ch);
        }
    }
    
protected:
    //filter processing method
    //input to be processed
    //channel to be processed
    SampleType processSample(SampleType input, size_t channel);
};
