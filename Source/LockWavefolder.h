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

template <typename SampleType>
class LockWavefolder{
    
public:
    
    LockWavefolder();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void initWavefolder(size_t samplesPerBlock, SampleType startingFold, SampleType startingOffset);
    
    template <typename ProcessContext>
    void process(const ProcessContext& context){
        
        if (context.isBypassed) {
            auto outputBlock = context.getOutputBlock();
            outputBlock.copyFrom (context.getInputBlock());
            return;
        }
        
        const auto& inputBlock = context.getInputBlock();
        mixer.setWetLatency(SampleType(0));
        mixer.pushDrySamples(inputBlock);
        auto ovBlock = oversampler.processSamplesUp(inputBlock);
        dsp::ProcessContextReplacing<SampleType> foldingContext(ovBlock);
        const auto& foldingInputBlock = foldingContext.getInputBlock();
        auto foldingOutputBlock = foldingContext.getOutputBlock();
        const auto numSamples = foldingOutputBlock.getNumSamples();
        const auto numChannels = foldingOutputBlock.getNumChannels();
        
        for (size_t n = 0; n < numSamples; ++n){
            
            updateSmoothers();
                    
            for (size_t ch = 0; ch < numChannels; ++ch)
                foldingOutputBlock.getChannelPointer (ch)[n] = processSample (foldingInputBlock.getChannelPointer (ch)[n]);
        }
        auto& outputBlock = context.getOutputBlock();
        oversampler.processSamplesDown(outputBlock);
        mixer.mixWetSamples(outputBlock);
    };
    
    SampleType getFold();
    void setFold(SampleType folding);
    
    SampleType getOffset();
    void setOffset(SampleType inputOffset);
    
    void setSampleRate(SampleType samplingFreq);
    
    void setMixProportion(SampleType mix);
    
    
private:
  
    SampleType sampleRate;
    dsp::Oversampling<SampleType> oversampler;
    
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> foldSmoother, offsetSmoother;
    void updateSmoothers();
    SampleType currentFold, fold, currentOffset, offset;
    SampleType foldMapping(SampleType x){return jmap(x, SampleType(1), SampleType(10));};
    SampleType offsetMapping(SampleType x){return jmap(x, SampleType(0), SampleType(0.8));};
    dsp::LookupTableTransform<SampleType> tanhLUT { [] (SampleType x) { return std::tanh (x); },
                                                         SampleType (-5), SampleType (5), 256 };
    
    dsp::DryWetMixer<SampleType> mixer { 10 };
    
    
    SampleType processSampleLWFOneStage(SampleType input);
    
    const SampleType alpha = SampleType(2)*SampleType(7.5)/SampleType(15);
    const SampleType beta = (SampleType(2)*SampleType(7.5) + SampleType(15))/(SampleType(0.025864)*SampleType(15));
    const SampleType delta = SampleType(7.5e+3)*SampleType(10e-17)/SampleType(0.025864);
    SampleType sign(SampleType x) { if(x > SampleType(0)){
            return SampleType(1);
        }else if (x < SampleType(0)){
            return SampleType(-1);
        }else{
            return SampleType(0);
        }
    };
    
    
protected:
    
    SampleType processSample(SampleType input);
    
};
