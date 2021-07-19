/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
CutTheMoogAudioProcessor::CutTheMoogAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
                        treeState(*this, nullptr, "PARAMETERS", createParameterLayout()),
       lockWavefolder(treeState),
       moogCatFilter(treeState)
#endif
{
    LookAndFeel::setDefaultLookAndFeel(&myLnf);
    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.setGuiValueTree(BinaryData::CutTheMoog_xml, BinaryData::CutTheMoog_xmlSize);
}

CutTheMoogAudioProcessor::~CutTheMoogAudioProcessor()
{
}

AudioProcessorValueTreeState::ParameterLayout CutTheMoogAudioProcessor::createParameterLayout() {

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (GAIN_ID, GAIN_NAME, -40.0f, 12.0f, -1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (TRIM_ID, TRIM_NAME, -8.0f, 8.0f, 0.0f));
    params.push_back (std::make_unique<AudioParameterBool> (BYPASS_ID, BYPASS_NAME, true));

    MoogCat::createParameterLayout(params);
    LockWavefolder::createParameterLayout(params);

    return { params.begin(), params.end() };

}

//==============================================================================
const String CutTheMoogAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CutTheMoogAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CutTheMoogAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CutTheMoogAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CutTheMoogAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CutTheMoogAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CutTheMoogAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CutTheMoogAudioProcessor::setCurrentProgram (int index)
{
}

const String CutTheMoogAudioProcessor::getProgramName (int index)
{
    return {};
}

void CutTheMoogAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void CutTheMoogAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();

    inputGain.prepare(spec);
    outputGain.prepare(spec);

    lockWavefolder.prepare(spec);
    moogCatFilter.prepare(spec);

    delayLine.prepare(spec);
    delayLine.setDelay(getLatency());
    setLatencySamples(roundToInt(getLatency()));
    bypass.prepare(spec, true, getLatency());
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

void CutTheMoogAudioProcessor::releaseResources()
{
    lockWavefolder.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CutTheMoogAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CutTheMoogAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;

    auto bypassParam = treeState.getRawParameterValue(BYPASS_ID);
    if (! bypass.processBlockIn(buffer, BypassProcessor::toBool(bypassParam))) {
        setLatencySamples(roundToInt(getLatency()));
        bypass.setDelayInSamples(getLatency());
        analyser->pushSamples(buffer);
        return;
    }
    inputGain.setGainDecibels(*treeState.getRawParameterValue(TRIM_ID) );
    outputGain.setGainDecibels(*treeState.getRawParameterValue(GAIN_ID));

    inputGain.process(buffer);

    lockWavefolder.process(buffer);
    moogCatFilter.process(buffer);

    // compensate bypass buffer delay
    setLatencySamples(roundToInt(getLatency()));
    bypass.setDelayInSamples(getLatency());

    outputGain.process(buffer);

    bypass.processBlockOut(buffer, BypassProcessor::toBool(bypassParam));
    analyser -> pushSamples(buffer);
    
}

void CutTheMoogAudioProcessor::processBlockBypassed(AudioBuffer<float> & buffer, MidiBuffer & midiMessages){
    
    ScopedNoDenormals noDenormals;
    
    dsp::AudioBlock<float> block(buffer);
    delayLine.setDelay(getLatency());
    setLatencySamples(roundToInt(getLatency()));
    delayLine.process(dsp::ProcessContextReplacing<float> (block));
}

//==============================================================================

void CutTheMoogAudioProcessor::initialiseBuilder(foleys::MagicGUIBuilder& builder)
{
    builder.registerJUCEFactories();
    builder.registerJUCELookAndFeels();

    builder.registerFactory ("PowerButton", &PowerButtonItem::factory);
    builder.registerLookAndFeel("Skeuomorphic", std::make_unique<foleys::Skeuomorphic>());
    builder.registerLookAndFeel("MYLNF", std::make_unique<OtherLookAndFeel>());
}


float CutTheMoogAudioProcessor::getLatency(){
    return lockWavefolder.getLatency();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CutTheMoogAudioProcessor();
}
