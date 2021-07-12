/*
  ==============================================================================

    OtherLookAndFeel.h
    Created: 10 Jan 2021 8:36:49pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    
    OtherLookAndFeel(){
        
        futura = juce::Typeface::createSystemTypefaceFor(BinaryData::FuturaHeavyfont_ttf, BinaryData::FuturaHeavyfont_ttfSize);
        
        futuraBold = juce::Typeface::createSystemTypefaceFor(BinaryData::FuturaBoldFont_ttf, BinaryData::FuturaBoldFont_ttfSize);
        
        setDefaultSansSerifTypeface(futura);
    }
    
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& font) override
    {
        return font.isBold() ? futuraBold : futura;
    }
    
    void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) override{
        
        float diameter_Pie = juce::jmin(width, height);
        float arcFactor = 0.9;
        float diameter = diameter_Pie - (1-arcFactor + 0.1)*diameter_Pie;
        float radius = diameter/2;
        float centreX = x + width/2;
        float centreY = y + height/2 + ((1-arcFactor)*diameter_Pie/4);
        float rx = centreX - radius;
        float ry = centreY - radius;
        
        float radius_Pie = diameter_Pie/2;
        float rx_Pie = centreX - radius_Pie;
        float ry_Pie = centreY - radius_Pie;
        
        float pointerLengthFactor = 0.38;
        float pointerLargeFactor = 0.05;
        float diameterInternal = diameter - radius*pointerLengthFactor*2;
        float radiusInternal = diameterInternal/2;
        float rx_internal = centreX - radiusInternal;
        float ry_internal = centreY - radiusInternal;
        
        float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));
        
        
        juce::Rectangle<float> dialArea(rx, ry, diameter, diameter);
        
        
        g.setColour(juce::Colour(0xff303031));
        //g.drawRect(dialArea);
        g.fillEllipse(dialArea);
        
        juce::Rectangle<float> insideDialArea(rx_internal, ry_internal, diameterInternal, diameterInternal);
        g.setColour(juce::Colour(0xff111111));
        g.fillEllipse(insideDialArea);
        
        g.setColour(juce::Colour(0xff30cde9));
        juce::Path dialTick;
        dialTick.addRectangle(0, -radius, radius*pointerLargeFactor, radius * pointerLengthFactor);
        g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        
        juce::Path dialArc;
        dialArc.addPieSegment(rx_Pie, ry_Pie, diameter_Pie, diameter_Pie, rotaryStartAngle, rotaryEndAngle, arcFactor);
        g.setColour(juce::Colour(0xffc3e1e6));
        g.fillPath(dialArc);
        dialArc.clear();
        
        dialArc.addPieSegment(rx_Pie, ry_Pie, diameter_Pie, diameter_Pie, rotaryStartAngle, angle, arcFactor);
        g.setColour(juce::Colour(0xff30cde9));
        g.fillPath(dialArc);
        
    }
    
private:
    
    juce::Typeface::Ptr futuraBold;
    juce::Typeface::Ptr futura;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OtherLookAndFeel)
};
