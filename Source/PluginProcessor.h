/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#define GAIN_ID "output"
#define GAIN_NAME "Output"
#define TRIM_ID "trim"
#define TRIM_NAME "Trim"
#define BYPASS_ID "bypass"
#define BYPASS_NAME "Bypass"

#include <JuceHeader.h>
#include "Processors/MoogCat.h"
#include "Processors/LockWavefolder.h"
#include "Processors/BypassProcessor.h"
#include "Processors/DCBlocker.h"
#include "Processors/GainProcessor.h"
#include "PowerButton.h"
#include "OtherLookAndFeel.h"

//==============================================================================
/**
*/
class CutTheMoogAudioProcessor  : public foleys::MagicProcessor
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
    
    //==============================================================================
    
    void initialiseBuilder(foleys::MagicGUIBuilder& builder) override;

private:

    AudioProcessorValueTreeState treeState;
    foleys::MagicPlotSource* analyser = nullptr;

    dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine{100};
    BypassProcessor bypass;
    float getLatency();
    
    OtherLookAndFeel myLnf;

    LockWavefolder lockWavefolder;
    MoogCat moogCatFilter;

    GainProcessor inputGain, outputGain;

    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CutTheMoogAudioProcessor)
};
