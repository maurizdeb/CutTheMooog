/*
  ==============================================================================

    LockWavefolder.h
    Created: 21 Nov 2020 5:18:46pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LWSolver.h"
#include "DCBlocker.h"

#define FOLDING_ID "folding"
#define FOLDING_NAME "Folding"
#define OFFSET_ID "offset"
#define OFFSET_NAME "Offset"
#define DRYWET_ID "drywet"
#define DRYWET_NAME "DryWet"

#define NUM_STEPS 500

class LockWavefolder{
    
public:
    
    LockWavefolder(AudioProcessorValueTreeState& vts);
    ~LockWavefolder();

    void prepare(const juce::dsp::ProcessSpec& spec);

    void reset();

    float getLatency() noexcept;

    void process(AudioBuffer<float>& buffer);
    
    float getFold() noexcept;
    void setFold(float folding) noexcept;
    
    float getOffset() noexcept;
    void setOffset(float inputOffset) noexcept;
    
    void setSampleRate(float samplingFreq) noexcept;
    
    void setMixProportion(float mix) noexcept;
    
    void releaseResources();

    static void createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>>& params);
    
private:
  
    float sampleRate;
    std::unique_ptr<dsp::Oversampling<float>> oversampler;
    
    SmoothedValue<float, ValueSmoothingTypes::Linear> foldSmoother, offsetSmoother;

    void updateSmoothers() noexcept;

    //params
    float currentFold = 0.5, currentOffset = 0.0;
    std::atomic<float>* foldParam = nullptr;
    std::atomic<float>* offsetParam = nullptr;
    std::atomic<float>* dwParam = nullptr;

    float foldMapping(float x) noexcept {return jmap<float>(x, 1.0, 10.0);}
    float offsetMapping(float x) noexcept {return jmap<float>(x, 0.0, 5.0);}

    dsp::LookupTableTransform<float> tanhLUT { [] (float x) { return std::tanh (x); },
                                                         -6.0, 6.0, 256 };

    DCBlocker dcBlocker[2];

    dsp::DryWetMixer<float> mixer { 20 };

    
    
    float processSampleLWFOneStage(float input) noexcept;
    
    const float alpha = 2 * 7.5f / 15;
    const float beta = ( 2 * 7.5f + 15) / ( 0.025864f * 15 );
    const float delta = 7.5e+3f * 10e-17f / 0.025864f;
    float sign(float x) { if(x > 0.0){
            return 1.0;
        }else if (x < 0.0){
            return -1.0;
        }else{
            return 0.0;
        }
    }

    void applyDCblock(dsp::AudioBlock<float>& buffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockWavefolder)
protected:
    
    float processSample(float input) noexcept;
    
};
