/*
  ==============================================================================

    LockWavefolder.cpp
    Created: 21 Nov 2020 5:18:46pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#include "LockWavefolder.h"

template <typename SampleType>
LockWavefolder<SampleType>::LockWavefolder(): oversampler(2,2,dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR, true, false){
    mixer.setMixingRule(dsp::DryWetMixingRule::linear);
    fold = SampleType(0.5);
    offset = SampleType(0);
}

template <typename SampleType>
void LockWavefolder<SampleType>::prepare(const juce::dsp::ProcessSpec &spec){
    setSampleRate(spec.sampleRate); //oversampling factor of 4 = 2^2
    foldSmoother.reset(sampleRate, SampleType(0.05));
    offsetSmoother.reset(sampleRate, SampleType(0.05));
    mixer.prepare(spec);
    
    reset();
}

template <typename SampleType>
void LockWavefolder<SampleType>::reset(){
    
    foldSmoother.setCurrentAndTargetValue(fold);
    offsetSmoother.setCurrentAndTargetValue(offset);
    mixer.reset();
}

template <typename SampleType>
void LockWavefolder<SampleType>::initWavefolder(size_t samplesPerBlock, SampleType startingFold, SampleType startingOffset){
    
    oversampler.initProcessing(samplesPerBlock);
    
    mixer.setWetLatency(oversampler.getLatencyInSamples());
    
    fold = startingFold;
    currentFold = foldMapping(fold);
    foldSmoother.setCurrentAndTargetValue(currentFold);
    
    offset = startingOffset;
    currentOffset = offsetMapping(offset);
    offsetSmoother.setCurrentAndTargetValue(currentOffset);
}

template <typename SampleType>
SampleType LockWavefolder<SampleType>::getLatency() noexcept{
    return oversampler.getLatencyInSamples();
}

template <typename SampleType>
SampleType LockWavefolder<SampleType>::getFold() noexcept {
    return fold;
}

template <typename SampleType>
void LockWavefolder<SampleType>::setFold(SampleType folding) noexcept {
    fold = folding;
    foldSmoother.setTargetValue(foldMapping(fold));
}

template <typename SampleType>
SampleType LockWavefolder<SampleType>::getOffset() noexcept {
    return offset;
}

template <typename SampleType>
void LockWavefolder<SampleType>::setOffset(SampleType inputOffset) noexcept {
    offset = inputOffset;
    offsetSmoother.setTargetValue(offsetMapping(offset));
}

template <typename SampleType>
void LockWavefolder<SampleType>::setSampleRate(SampleType samplingFreq) noexcept {
    sampleRate = samplingFreq;
}

template <typename SampleType>
void LockWavefolder<SampleType>::setMixProportion(SampleType mix) noexcept {
    mixer.setWetMixProportion(mix);
}

template <typename SampleType>
void LockWavefolder<SampleType>::updateSmoothers() noexcept {
    currentFold = foldSmoother.getNextValue();
    currentOffset = offsetSmoother.getNextValue();
}

template <typename SampleType>
SampleType LockWavefolder<SampleType>::processSampleLWFOneStage(SampleType input) noexcept {
    const auto sign_in = sign(input);
    const auto delta_log = SampleType(LWSolver::logf_approx(delta));
    const auto inputLambert = delta_log + sign_in*beta*input;
    const auto lambert_value = SampleType(LWSolver::omega4(inputLambert));
    const auto rightSide = sign_in*SampleType(0.025864)*lambert_value;
    const auto leftSide = alpha*input;
    
    return leftSide - rightSide;
}

template <typename SampleType>
SampleType LockWavefolder<SampleType>::processSample(SampleType input, size_t channel) noexcept{
    
    const auto val1 = (currentFold*input + currentOffset);
    const auto input_stage1 = val1/SampleType(3);
    const auto input_stage2 = processSampleLWFOneStage(input_stage1);
    const auto input_stage3 = processSampleLWFOneStage(input_stage2);
    const auto input_stage4 = processSampleLWFOneStage(input_stage3);
    const auto output_stage4 = processSampleLWFOneStage(input_stage4);
    const auto output = output_stage4*SampleType(3);
    
    return tanhLUT(output);
}

template <typename SampleType>
void LockWavefolder<SampleType>::releaseResources(){
    oversampler.reset();
}

template class LockWavefolder<float>;
template class LockWavefolder<double>;
