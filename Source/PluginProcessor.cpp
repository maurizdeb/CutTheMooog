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
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
                        treeState(*this, nullptr, "PARAMETERS", {
                            //cutoff frequency controlled in a logarithmic way
                            std::make_unique<AudioParameterFloat>(FREQUENCY_ID, FREQUENCY_NAME, NormalisableRange<float>(20, 20000, 0.01, 0.1989), 800),
                            //k control the resonance and must be less than one to keep the filter stable
                            std::make_unique<AudioParameterFloat>(RESONANCE_ID, RESONANCE_NAME, NormalisableRange<float>(0, 1, 0.001, 0.8), 0.0f),
                            std::make_unique<AudioParameterFloat>(GAIN_ID, GAIN_NAME, -40.0f, 12.0f, -1.0f),
                            //r type of the filter, morphing parameter
                            std::make_unique<AudioParameterFloat>(MORPHING_ID, MORPHING_NAME, NormalisableRange<float>(0, 1, 0.001), 0.564),
                            std::make_unique<AudioParameterFloat>(FOLDING_ID, FOLDING_NAME, NormalisableRange<float>(0, 1, 0.001), 0.5),
                            std::make_unique<AudioParameterFloat>(OFFSET_ID, OFFSET_NAME, NormalisableRange<float>(0, 1, 0.001), 0.0),
                            std::make_unique<AudioParameterFloat>(TRIM_ID, TRIM_NAME, -24.0f, 24.0f, 0.0f),
                            std::make_unique<AudioParameterFloat>(DRYWET_ID, DRYWET_NAME, 0.0f, 1.0f, 0.5f)
                        })
#endif
{
    //add listener to the value tree state
    treeState.addParameterListener(FREQUENCY_ID, this);
    treeState.addParameterListener(RESONANCE_ID, this);
    treeState.addParameterListener(GAIN_ID, this);
    treeState.addParameterListener(MORPHING_ID, this);
    treeState.addParameterListener(FOLDING_ID, this);
    treeState.addParameterListener(OFFSET_ID, this);
    treeState.addParameterListener(TRIM_ID, this);
    treeState.addParameterListener(DRYWET_ID, this);
    
    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing (analyser);
}

CutTheMoogAudioProcessor::~CutTheMoogAudioProcessor()
{
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
    //filterMoogCat.prepare(spec);
    //lockFolder.prepare(spec);
    processorChain.prepare(spec);
    
    //update of the filter
    //Get and update treeStateValues
    initTrimParams();
    initFolderParams(samplesPerBlock);
    initFilterParams();
    initOutput();
    //oldGain = *currentGain;
    
    
    // Reset the filter states
    //filterMoogCat.reset();
    analyser->prepareToPlay (sampleRate, samplesPerBlock);
}

void CutTheMoogAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

void CutTheMoogAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    //int bufferLength = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    updateTrimParams();
    updateFolderParams();
    updateFilterParams();
    updateOutputParams();
        
    dsp::AudioBlock<float> block(buffer);
    //lockFolder.process(dsp::ProcessContextReplacing<float> (block));
    //filterMoogCat.process(dsp::ProcessContextReplacing<float> (block));
    processorChain.process(dsp::ProcessContextReplacing<float> (block));
    
    analyser -> pushSamples(buffer);
    
}

//==============================================================================
bool CutTheMoogAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* CutTheMoogAudioProcessor::createEditor()
{
    //return new CutTheMoogAudioProcessorEditor (*this, treeState);
    return new foleys::MagicPluginEditor(magicState, BinaryData::CutTheMoog_xml, BinaryData::CutTheMoog_xmlSize);
}

//==============================================================================
void CutTheMoogAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = treeState.copyState();
    std::unique_ptr<XmlElement> fileXml(state.createXml());
    copyXmlToBinary(*fileXml, destData);
}

void CutTheMoogAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr){
        if (xmlState -> hasTagName(treeState.state.getType())){
            treeState.replaceState(ValueTree::fromXml(*xmlState));
        }
    }
}

void CutTheMoogAudioProcessor::parameterChanged (const String &treeWhosePropertyHasChanged, float newValue){
    
    if (treeWhosePropertyHasChanged == FREQUENCY_ID || treeWhosePropertyHasChanged == RESONANCE_ID || treeWhosePropertyHasChanged == MORPHING_ID) {
        filterShouldUpdate = true;
    }
    if (treeWhosePropertyHasChanged == GAIN_ID) {
        gainShouldUpdate = true;
    }
    if (treeWhosePropertyHasChanged == FOLDING_ID || treeWhosePropertyHasChanged == OFFSET_ID || treeWhosePropertyHasChanged == DRYWET_ID) {
        foldShouldUpdate = true;
    }
    if (treeWhosePropertyHasChanged == TRIM_ID) {
        trimShouldUpdate = true;
    }
}

void CutTheMoogAudioProcessor::updateFilterParams(){
    
    if (filterShouldUpdate){
        //Get and update treeStateValues
        auto currentFrequency = treeState.getRawParameterValue(FREQUENCY_ID);
        auto currentResonance = treeState.getRawParameterValue(RESONANCE_ID);
        auto currentFilterType = treeState.getRawParameterValue(MORPHING_ID);
        auto& filter = processorChain.template get<filterIndex>();
        filter.update(*currentFrequency, *currentResonance, *currentFilterType);
        filterShouldUpdate = false;
    }
}

void CutTheMoogAudioProcessor::initFilterParams(){
    
    //Get and update treeStateValues
    auto currentFrequency = treeState.getRawParameterValue(FREQUENCY_ID);
    auto currentResonance = treeState.getRawParameterValue(RESONANCE_ID);
    auto currentFilterType = treeState.getRawParameterValue(MORPHING_ID);
    auto& filter = processorChain.template get<filterIndex>();
    filter.initFilter(*currentFrequency, *currentResonance, *currentFilterType);
}

void CutTheMoogAudioProcessor::initOutput(){
    auto currentGain = treeState.getRawParameterValue(GAIN_ID);
    auto& gainProc = processorChain.template get<outputIndex>();
    gainProc.setRampDurationSeconds(0.05);
    gainProc.setGainDecibels(*currentGain);
}

void CutTheMoogAudioProcessor::updateOutputParams(){
    
    if (gainShouldUpdate){
        auto currentGain = treeState.getRawParameterValue(GAIN_ID);
        auto& gainProc = processorChain.template get<outputIndex>();
        gainProc.setGainDecibels(*currentGain);
        gainShouldUpdate = false;
    }
}

void CutTheMoogAudioProcessor::initFolderParams(int samplesPerBlock){
    
    auto currentFold = treeState.getRawParameterValue(FOLDING_ID);
    auto currentOffset = treeState.getRawParameterValue(OFFSET_ID);
    auto currentDryWet = treeState.getRawParameterValue(DRYWET_ID);
    auto& folder = processorChain.template get<folderIndex>();
    folder.initWavefolder(static_cast<size_t> (samplesPerBlock), *currentFold, *currentOffset);
    folder.setMixProportion(*currentDryWet);
}

void CutTheMoogAudioProcessor::updateFolderParams(){
    
    if (foldShouldUpdate) {
        auto currentFold = treeState.getRawParameterValue(FOLDING_ID);
        auto currentOffset = treeState.getRawParameterValue(OFFSET_ID);
        auto currentDryWet = treeState.getRawParameterValue(DRYWET_ID);
        auto& folder = processorChain.template get<folderIndex>();
        folder.setFold(*currentFold);
        folder.setOffset(*currentOffset);
        folder.setMixProportion(*currentDryWet);
        foldShouldUpdate = false;
    }
}

void CutTheMoogAudioProcessor::initTrimParams(){
    
    auto currentGain = treeState.getRawParameterValue(TRIM_ID);
    auto& gainProc = processorChain.template get<trimIndex>();
    gainProc.setRampDurationSeconds(0.05);
    gainProc.setGainDecibels(*currentGain);
}

void CutTheMoogAudioProcessor::updateTrimParams(){
    
    if (trimShouldUpdate){
        auto currentGain = treeState.getRawParameterValue(TRIM_ID);
        auto& gainProc = processorChain.template get<trimIndex>();
        gainProc.setGainDecibels(*currentGain);
        trimShouldUpdate = false;
    }
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CutTheMoogAudioProcessor();
}
