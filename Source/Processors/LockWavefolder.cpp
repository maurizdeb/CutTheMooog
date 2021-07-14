/*
  ==============================================================================

    LockWavefolder.cpp
    Created: 21 Nov 2020 5:18:46pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#include "LockWavefolder.h"


LockWavefolder::LockWavefolder(AudioProcessorValueTreeState& vts){

    foldParam = vts.getRawParameterValue(FOLDING_ID);
    offsetParam = vts.getRawParameterValue(OFFSET_ID);
    dwParam = vts.getRawParameterValue(DRYWET_ID);

    mixer.setMixingRule(dsp::DryWetMixingRule::linear);
    foldSmoother.reset(NUM_STEPS);
    offsetSmoother.reset(NUM_STEPS);
    setSampleRate(44100.0f);
    setFold(*foldParam);
    setOffset(*offsetParam);
    setMixProportion(*dwParam);

    oversampler = std::make_unique<dsp::Oversampling<float>>(2, 2, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
}

LockWavefolder::~LockWavefolder() {

}

void LockWavefolder::createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>> &params) {

    params.push_back(std::make_unique<AudioParameterFloat>(FOLDING_ID, FOLDING_NAME, NormalisableRange<float>(0, 1, 0.001f), 0.5f));
    params.push_back(std::make_unique<AudioParameterFloat>(OFFSET_ID, OFFSET_NAME, NormalisableRange<float>(0, 1, 0.001f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(DRYWET_ID, DRYWET_NAME, 0.0f, 1.0f, 0.5f));
}

void LockWavefolder::prepare(const juce::dsp::ProcessSpec &spec){
    setSampleRate((float) spec.sampleRate);
    mixer.prepare(spec);
    oversampler -> initProcessing((size_t) spec.maximumBlockSize);
    dcBlocker[0].prepare(spec.sampleRate, 30.0f);
    dcBlocker[1].prepare(spec.sampleRate, 30.0f);

    reset();
}


void LockWavefolder::reset(){
    
    foldSmoother.skip(NUM_STEPS);
    offsetSmoother.skip(NUM_STEPS);
    mixer.reset();
}

float LockWavefolder::getLatency() noexcept{
    return oversampler -> getLatencyInSamples();
}


float LockWavefolder::getFold() noexcept {
    return *foldParam;
}

void LockWavefolder::setFold(float folding) noexcept {
    foldSmoother.setTargetValue(foldMapping(folding));
}

float LockWavefolder::getOffset() noexcept {
    return *offsetParam;
}

void LockWavefolder::setOffset(float inputOffset) noexcept {
    offsetSmoother.setTargetValue(offsetMapping(inputOffset));
}

void LockWavefolder::setSampleRate(float samplingFreq) noexcept {
    sampleRate = samplingFreq;
}

void LockWavefolder::setMixProportion(float mix) noexcept {
    mixer.setWetMixProportion(mix);
}

void LockWavefolder::updateSmoothers() noexcept {
    currentFold = foldSmoother.getNextValue();
    currentOffset = offsetSmoother.getNextValue();
}

float LockWavefolder::processSampleLWFOneStage(float input) noexcept {
    const float sign_in = sign (input);
    const float inputLambert = delta_log + sign_in * beta * input;
    const float lambert_value = LWSolver::omega4 (inputLambert);
    const float rightSide = sign_in * 0.025864f * lambert_value;
    const float leftSide = alpha * input;
    
    return leftSide - rightSide;
}

float LockWavefolder::processSample(float input) noexcept{
    
    const auto val1 = ( currentFold * input + currentOffset );
    const auto input_stage1 = val1 / 3;
    const auto input_stage2 = processSampleLWFOneStage (input_stage1);
    const auto input_stage3 = processSampleLWFOneStage (input_stage2);
    const auto input_stage4 = processSampleLWFOneStage (input_stage3);
    const auto output_stage4 = processSampleLWFOneStage (input_stage4);
    const auto output = output_stage4 * 3;
    
    return tanhLUT(output);
}

void LockWavefolder::releaseResources(){
    oversampler -> reset();
}

void LockWavefolder::applyDCblock(dsp::AudioBlock<float>& buffer) {

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        dcBlocker[ch].processBlock (buffer.getChannelPointer (ch), buffer.getNumSamples());
}

void LockWavefolder::process(AudioBuffer<float>& buffer){

    setFold(*foldParam);
    setOffset(*offsetParam);
    setMixProportion(*dwParam);

    dsp::AudioBlock<float> block (buffer);
    mixer.pushDrySamples(block);
    dsp::AudioBlock<float> ovBlock = oversampler -> processSamplesUp(block);

    for (size_t n = 0; n < ovBlock.getNumSamples(); ++n){

        updateSmoothers();

        for (size_t ch = 0; ch < ovBlock.getNumChannels(); ++ch)
            ovBlock.getChannelPointer (ch)[n] = processSample (ovBlock.getChannelPointer (ch)[n]);
    }
    oversampler -> processSamplesDown(block);

    applyDCblock(block);

    mixer.mixWetSamples(block);
}


