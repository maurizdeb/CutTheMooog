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
    
    
    //vector of states
    static constexpr size_t numStates = 4;
    std::vector<std::array<SampleType, numStates>> state;
    
    void setNumChannels (size_t newValue)   { state.resize (newValue); }
    
    //State-space matrices initialization
    void updateSmoothersAndStateSpace() noexcept;
    
    SampleType resonanceMap(SampleType resonance) noexcept {return jmap(resonance, SampleType(0), SampleType(0.99));}
    SampleType filterTypeMap(SampleType map) noexcept {return jmap(map, SampleType(0.4), SampleType(1.5)); }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoogCat);
    
public:
    
    //constructor and destructor methods
    MoogCat();
    ~MoogCat();
    
    //cutoff getter and setter
    void setCutoffFrequency(SampleType cutoff_frequency) noexcept;
    
    //resonance getter and setter
    void setResonance(SampleType resonance) noexcept;
    
    
    // coeeficient of filter type getter and setter
    void setFilterType(SampleType r) noexcept;
    
    
    //sampleRate getter and setter
    void setSampleRate(SampleType sampleRate) noexcept;
    
    size_t getNumChannels() const noexcept       { return state.size(); }
    
    //initialize the filter
    void initFilter() noexcept;
    void initFilter(SampleType cutoff, SampleType resonance, SampleType type) noexcept ;
    
    //update the filter
    void update(SampleType cutoff, SampleType resonance, SampleType type) noexcept;
    
    //prepare the filter
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    //reset the filter
    void reset();
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept {
        
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
    SampleType processSample(SampleType input, size_t channel) noexcept;
};
