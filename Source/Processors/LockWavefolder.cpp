/*
  ==============================================================================

    LockWavefolder.cpp
    Created: 21 Nov 2020 5:18:46pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#include "LockWavefolder.h"


LockWavefolder::LockWavefolder(AudioProcessorValueTreeState& vts): stateADAA(2), stateTanh(2), stateFunADAA(2), stateFunTanh(2){

    foldParam = vts.getRawParameterValue(FOLDING_ID);
    offsetParam = vts.getRawParameterValue(OFFSET_ID);
    dwParam = vts.getRawParameterValue(DRYWET_ID);
    osParam = vts.getRawParameterValue(OV_ID);
    adaaParam = vts.getRawParameterValue(ADAA_ID);

    mixer.setMixingRule(dsp::DryWetMixingRule::linear);
    foldSmoother.reset(NUM_STEPS);
    offsetSmoother.reset(NUM_STEPS);
    setSampleRate(44100.0f);
    setFold(*foldParam);
    setOffset(*offsetParam);
    setMixProportion(*dwParam);

    for (int i = 0; i < 4; ++i) {
        oversampler[i] = std::make_unique<dsp::Oversampling<float>>(2, i + 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    }
    curOs = (int) *osParam;
    prevOs = curOs;

    curADAA = (int) *adaaParam;
    prevADAA = curADAA;
}

LockWavefolder::~LockWavefolder() {

}

void LockWavefolder::createParameterLayout(std::vector<std::unique_ptr<RangedAudioParameter>> &params) {

    params.push_back(std::make_unique<AudioParameterFloat>(FOLDING_ID, FOLDING_NAME, NormalisableRange<float>(0, 1, 0.001f), 0.5f));
    params.push_back(std::make_unique<AudioParameterFloat>(OFFSET_ID, OFFSET_NAME, NormalisableRange<float>(0, 1, 0.001f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(DRYWET_ID, DRYWET_NAME, 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<AudioParameterChoice>(OV_ID, OV_NAME, StringArray{"2x", "4x", "8x", "16x"}, 0));
    params.push_back(std::make_unique<AudioParameterChoice>(ADAA_ID, ADAA_NAME, StringArray{"No", "Yes"}, 1));
}

void LockWavefolder::prepare(const juce::dsp::ProcessSpec &spec){
    setSampleRate((float) spec.sampleRate);
    setNumChannels(spec.numChannels);
    mixer.prepare(spec);
    for (int i = 0; i < 4; ++i) {
        oversampler[i] -> initProcessing((size_t) spec.maximumBlockSize);
    }
    prevOs = curOs;
    dcBlocker[0].prepare(spec.sampleRate, 30.0f);
    dcBlocker[1].prepare(spec.sampleRate, 30.0f);

    reset();
}


void LockWavefolder::reset(){

    resetSmootherAndLatency();
    mixer.reset();

    for (auto&s:stateADAA) {
        s.fill(0.0f);
    }

    for (auto&s:stateFunADAA) {
        s.fill(0.0f);
    }

    for (auto&s:stateTanh) {
        s = 0.0f;
    }

    for (auto&s:stateFunTanh) {
        s = 0.0f;
    }
}

void LockWavefolder::resetSmootherAndLatency(){
    foldSmoother.skip(NUM_STEPS);
    offsetSmoother.reset((int) NUM_STEPS* pow(2, curOs));
    mixer.setWetLatency(getLatency());
}

float LockWavefolder::getLatency() noexcept{
    float adaaLatency = curADAA ? 0.5f * 5 / oversampler[curOs] -> getOversamplingFactor() : 0.0f;
    return oversampler[curOs] -> getLatencyInSamples() + adaaLatency;
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

float LockWavefolder::processSampleLWFOneStage(float input, size_t channel, size_t stage) noexcept {
    auto& s = stateADAA[channel];
    auto& sFun = stateFunADAA[channel];

    bool illCondition = std::abs(input - s[stage]) < tolerance;
    float adaaFun = computeAntiderivative(input);

    float out = illCondition ? computeLWFunction(0.5f * (input + s[stage])) : (adaaFun - sFun[stage]) / (input - s[stage]);

    sFun[stage] = adaaFun;
    s[stage] = input;
    return out;
}

float LockWavefolder::processSampleLWFOneStage(float input) noexcept {
    return computeLWFunction(input);
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

float LockWavefolder::processTanh(float input, size_t channel) noexcept {

    auto& s = stateTanh[channel];
    auto& sFun = stateFunTanh[channel];

    bool illCondition = std::abs(input - s) < tolerance;
    float adaaFun = logCoshLUT(input);

    float out = illCondition ? tanhLUT(0.5f * (input + s)) : (adaaFun - sFun) / (input - s);
    s = input;
    sFun = adaaFun;
    return out;
}

float LockWavefolder::computeAntiderivative(float input) noexcept {
    const float sign_in = sign (input);
    const float inputLambert = delta_log + sign_in * beta * input;
    const float lambert_value = LWSolver::omega4 (inputLambert);
    const float leftSide = alpha_half * input * input;
    const float rightSide = gamma * lambert_value * (lambert_value + 2);

    return leftSide - rightSide;
}

float LockWavefolder::computeLWFunction(float input) noexcept {
    const float sign_in = sign(input);
    const float inputLambert = delta_log + sign_in * beta * input;
    const float lambert_value = LWSolver::omega4(inputLambert);
    const float rightSide = sign_in * 0.025864f * lambert_value;
    const float leftSide = alpha*input;

    return leftSide - rightSide;
}

float LockWavefolder::processSample(float input, size_t channel) noexcept{
    
    const float val1 = ( currentFold * input + currentOffset );
    const float input_stage1 = val1 / 3;
    const float input_stage2 = processSampleLWFOneStage (input_stage1, channel, 0);
    const float input_stage3 = processSampleLWFOneStage (input_stage2, channel, 1);
    const float input_stage4 = processSampleLWFOneStage (input_stage3, channel, 2);
    const float output_stage4 = processSampleLWFOneStage (input_stage4, channel, 3);
    const float output = output_stage4 * 3;
    
    return processTanh(output, channel);
}

void LockWavefolder::releaseResources(){
    for (int i = 0; i < 4; ++i) {
        oversampler[i]->reset();
    }
}

void LockWavefolder::applyDCblock(dsp::AudioBlock<float>& buffer) {

    for (size_t ch = 0; ch < buffer.getNumChannels(); ++ch)
        dcBlocker[ch].processBlock (buffer.getChannelPointer (ch), buffer.getNumSamples());
}

void LockWavefolder::process(AudioBuffer<float>& buffer){

    setOversampling();
    setADAA();
    setFold(*foldParam);
    setOffset(*offsetParam);
    setMixProportion(*dwParam);

    dsp::AudioBlock<float> block (buffer);
    mixer.pushDrySamples(block);
    dsp::AudioBlock<float> ovBlock = oversampler[curOs] -> processSamplesUp(block);

    if (curADAA) {
        for (size_t n = 0; n < ovBlock.getNumSamples(); ++n) {

            updateSmoothers();

            for (size_t ch = 0; ch < ovBlock.getNumChannels(); ++ch)
                ovBlock.getChannelPointer(ch)[n] = processSample(ovBlock.getChannelPointer(ch)[n], ch);
        }
    }
    else{
        for (size_t n = 0; n < ovBlock.getNumSamples(); ++n) {

            updateSmoothers();

            for (size_t ch = 0; ch < ovBlock.getNumChannels(); ++ch)
                ovBlock.getChannelPointer(ch)[n] = processSample(ovBlock.getChannelPointer(ch)[n]);
        }
    }
    oversampler[curOs] -> processSamplesDown(block);

    applyDCblock(block);

    mixer.mixWetSamples(block);
}

void LockWavefolder::setNumChannels(size_t channels) {
    stateADAA.resize(channels);
    stateTanh.resize(channels);
    stateFunADAA.resize(channels);
    stateFunTanh.resize(channels);
}

void LockWavefolder::setOversampling() noexcept {

    curOs = (int) *osParam;
    if (curOs != prevOs) {
        resetSmootherAndLatency();
        prevOs = curOs;
    }
}

void LockWavefolder::setADAA() noexcept {

    curADAA = (int) *adaaParam;
    if (curADAA != prevADAA) {
        resetSmootherAndLatency();
        prevADAA = curADAA;
    }
}


