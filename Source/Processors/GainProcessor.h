#pragma once

#include <JuceHeader.h>

class GainProcessor
{
public:
    GainProcessor() {}

    void prepare (dsp::ProcessSpec /*spec*/)
    {
        oldGain = curGain;
    }

    void process (AudioBuffer<float>& buffer)
    {
        if (curGain != oldGain)
        {
            buffer.applyGainRamp (0, buffer.getNumSamples(), oldGain, curGain);
            oldGain = curGain;
            return;
        }

        buffer.applyGain (curGain);
    }

    void setGain (float gain)
    {
        if (gain == curGain)
            return;

        oldGain = curGain;
        curGain = gain;
    }

    void setGainDecibels(float gainDB){

        setGain(Decibels::decibelsToGain(gainDB));
    }

    float getGain() const { return curGain; }

    float getGainDecibels() const { return Decibels::gainToDecibels(curGain); }

private:
    float curGain = 1.0f;
    float oldGain = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};