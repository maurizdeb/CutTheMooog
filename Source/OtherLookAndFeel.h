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
        float arcFactor = 0.9f;
        float diameter = diameter_Pie - (1-arcFactor + 0.1f) * diameter_Pie;
        float radius = diameter/2;
        float centreX = x + width / 2.f;
        float centreY = y + height / 2.f + ((1-arcFactor) * diameter_Pie / 4.f);
        float rx = centreX - radius;
        float ry = centreY - radius;
        
        float radius_Pie = diameter_Pie / 2.f;
        float rx_Pie = centreX - radius_Pie;
        float ry_Pie = centreY - radius_Pie;
        
        float pointerLengthFactor = 0.38f;
        float pointerLargeFactor = 0.05f;
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

    juce::Colour findPopupColour (int colourId, juce::Component* target)
    {
        if (target)
            return target->findColour (colourId);

        return findColour (colourId);
    }

    void drawPopupMenuItemWithOptions (juce::Graphics& g, const juce::Rectangle<int>& area,
                                                    const bool isHighlighted,
                                                    const juce::PopupMenu::Item& item,
                                                    const juce::PopupMenu::Options& options) override {
        auto textColour = findPopupColour(juce::PopupMenu::textColourId, options.getTargetComponent());

        if (item.isSeparator) {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt(((float) r.getHeight() * 0.5f) - 0.5f));
            g.setColour(textColour.withAlpha(0.3f));
            g.fillRect(r.removeFromTop(1));
        } else {
            auto r = area.reduced(1);

            if (isHighlighted && item.isEnabled) {
                g.setColour(findPopupColour(juce::PopupMenu::highlightedBackgroundColourId,
                                            options.getTargetComponent()));
                g.fillRect(r);

                g.setColour(findPopupColour(juce::PopupMenu::highlightedTextColourId,
                                            options.getTargetComponent()));
            } else {
                g.setColour(textColour.withMultipliedAlpha(item.isEnabled ? 1.0f : 0.5f));
            }

            r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

            auto font = getPopupMenuFont();

            auto maxFontHeight = (float) r.getHeight() / 1.3f;

            if (font.getHeight() > maxFontHeight)
                font.setHeight(maxFontHeight);

            g.setFont(font);

            auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

            if (item.image != nullptr) {
                item.image->drawWithin(g, iconArea,
                                       juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                                       1.0f);
                r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
            } else if (item.isTicked) {
                auto tick = getTickShape(1.0f);
                g.fillPath(tick,
                           tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
            }

            if (item.subMenu.get() != nullptr) {
                auto arrowH = 0.6f * getPopupMenuFont().getAscent();

                auto x = static_cast<float> (r.removeFromRight((int) arrowH).getX());
                auto halfH = static_cast<float> (r.getCentreY());

                juce::Path path;
                path.startNewSubPath(x, halfH - arrowH * 0.5f);
                path.lineTo(x + arrowH * 0.6f, halfH);
                path.lineTo(x, halfH + arrowH * 0.5f);

                g.strokePath(path, juce::PathStrokeType(2.0f));

                if (isHighlighted && item.isEnabled) {
                    g.setColour(findPopupColour(juce::PopupMenu::highlightedBackgroundColourId,
                                                options.getTargetComponent()));
                    g.fillRect(r);

                    g.setColour(findPopupColour(juce::PopupMenu::highlightedTextColourId,
                                                options.getTargetComponent()));
                } else {
                    g.setColour(textColour.withMultipliedAlpha(item.isEnabled ? 1.0f : 0.5f));
                }
            }

            r.removeFromRight(3);
            g.drawFittedText(item.text, r, juce::Justification::centredLeft, 1);

            if (item.shortcutKeyDescription.isNotEmpty()) {
                auto f2 = font;
                f2.setHeight(f2.getHeight() * 0.75f);
                f2.setHorizontalScale(0.95f);
                g.setFont(f2);

                g.drawText(item.shortcutKeyDescription, r, juce::Justification::centredRight, true);
            }
        }
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        juce::Rectangle<int> boxBounds (0, 0, width, height);

        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

        g.setColour (box.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (1, 1,
                         box.getWidth() - 2,
                         box.getHeight() - 2);

        label.setFont (getComboBoxFont (box));
    }
private:
    
    juce::Typeface::Ptr futuraBold;
    juce::Typeface::Ptr futura;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OtherLookAndFeel)
};
