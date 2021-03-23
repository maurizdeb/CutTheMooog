/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#define FREQUENCY_ID "frequency"
#define FREQUENCY_NAME "Frequency"
#define RESONANCE_ID "resonance"
#define RESONANCE_NAME "Resonance"
#define GAIN_ID "output"
#define GAIN_NAME "Output"
#define MORPHING_ID "morphing"
#define MORPHING_NAME "Morphing"
#define FOLDING_ID "folding"
#define FOLDING_NAME "Folding"
#define OFFSET_ID "offset"
#define OFFSET_NAME "Offset"
#define TRIM_ID "trim"
#define TRIM_NAME "Trim"
#define DRYWET_ID "drywet"
#define DRYWET_NAME "DryWet"
#define BYPASS_ID "bypass"
#define BYPASS_NAME "Bypass"

#include <JuceHeader.h>
#include "Processors/MoogCat.h"
#include "Processors/LockWavefolder.h"
#include "Processors/BypassProcessor.h"
#include "Processors/DCBlocker.h"
#include "PowerButton.h"
#include "OtherLookAndFeel.h"

//==============================================================================
/**
*/
class CutTheMoogAudioProcessor  : public AudioProcessor,
                                public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    CutTheMoogAudioProcessor();
    ~CutTheMoogAudioProcessor();
    
    

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void processBlockBypassed (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    
    

private:
    
    //istance of the filter
    //MoogCat<float> filterMoogCat;
    
    // state of the parameters: cutoff frequency and resonance
    AudioProcessorValueTreeState treeState;
    foleys::MagicProcessorState magicState { *this, treeState };
    foleys::MagicPlotSource* analyser = nullptr;
    
    enum {
        trimIndex,
        folderIndex,
        filterIndex,
        outputIndex
    };
    
    juce::dsp::ProcessorChain<juce::dsp::Gain<float>,LockWavefolder<float>, MoogCat<float>, juce::dsp::Gain<float>> processorChain;
    dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Thiran> delayLine{20};
    BypassProcessor bypass;
    float getLatency();
    
    std::atomic<bool> filterShouldUpdate {false};
    std::atomic<bool> gainShouldUpdate {false};
    std::atomic<bool> foldShouldUpdate {false};
    std::atomic<bool> trimShouldUpdate {false};
    
    void parameterChanged (const String &treeWhosePropertyHasChanged, float newValue) override;
    
    void updateFilterParams();
    void initFilterParams();
    
    void initFolderParams(int SamplesPerBlock);
    void updateFolderParams();
    
    void updateOutputParams();
    void initOutput();
    
    void initTrimParams();
    void updateTrimParams();
    
    OtherLookAndFeel myLnf;
    
    DCBlocker dcBlockers[2];
    void applyDCblock(AudioBuffer<float> buffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CutTheMoogAudioProcessor)
};
