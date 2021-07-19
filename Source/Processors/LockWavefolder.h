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
#define OV_ID "oversampling"
#define OV_NAME "Oversampling"
#define ADAA_ID "antialiasing"
#define ADAA_NAME "Antialiasing"

#define NUM_STEPS 2205

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

    void setOversampling() noexcept;
    void setADAA() noexcept;
    
    void setSampleRate(float samplingFreq) noexcept;
    void setNumChannels(size_t channels);
    
    void setMixProportion(float mix) noexcept;
    
    void releaseResources();

    static void createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>>& params);
    
private:
  
    float sampleRate;
    std::unique_ptr<dsp::Oversampling<float>> oversampler[4];
    
    SmoothedValue<float, ValueSmoothingTypes::Linear> foldSmoother, offsetSmoother;

    //params
    float currentFold = 0.5, currentOffset = 0.0;
    std::atomic<float>* foldParam = nullptr;
    std::atomic<float>* offsetParam = nullptr;
    std::atomic<float>* dwParam = nullptr;
    std::atomic<float>* osParam = nullptr;
    std::atomic<float>* adaaParam = nullptr;
    int prevOs = 0, curOs = 0, curADAA = 1, prevADAA = 1;

    static constexpr float alpha = 2 * 7.5f / 15;
    static constexpr float alpha_half = alpha / 2;
    static constexpr float beta = ( 2 * 7.5f + 15) / ( 0.025864f * 15 );
    static constexpr float delta = 7.5e+3f * 10e-17f / 0.025864f;
    const float delta_log = LWSolver::log_approx (delta);
    static constexpr float gamma = 0.025864f / (2 * beta);
    static constexpr float tolerance = 1e-2f;
    float sign(float x) { return (0 < x) - (x < 0); }
    std::vector<std::array<float, 4>> stateADAA, stateFunADAA;
    std::vector<float> stateTanh, stateFunTanh;

    DCBlocker dcBlocker[2];

    dsp::DryWetMixer<float> mixer { 20 };

    float foldMapping(float x) noexcept {return jmap<float>(x, 1.0, 10.0);}
    float offsetMapping(float x) noexcept {return jmap<float>(x, 0.0, 5.0);}

    dsp::LookupTableTransform<float> tanhLUT { [] (float x) { return std::tanh (x); },
                                                         -6.0, 6.0, 256 };

    dsp::LookupTableTransform<float> logCoshLUT { [] (float x) { return std::log(std::cosh(x)); },
                                               -6.0, 6.0, 256 };

    void updateSmoothers() noexcept;
    void resetSmootherAndLatency();
    
    float processSampleLWFOneStage(float input, size_t channel, size_t stage) noexcept;
    float processSampleLWFOneStage(float input) noexcept;

    float computeAntiderivative(float input) noexcept;
    float computeLWFunction(float input) noexcept;
    float processTanh(float input, size_t channel) noexcept;

    void applyDCblock(dsp::AudioBlock<float>& buffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockWavefolder)
protected:
    
    float processSample(float input, size_t channel) noexcept;
    float processSample(float input) noexcept;
};
