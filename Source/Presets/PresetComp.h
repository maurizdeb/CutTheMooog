#pragma once
#include "PresetManager.h"
#include "../PluginProcessor.h"

class PresetComp : public Component,
                   private PresetManager::Listener
{
public:

    enum ColourIDs{
        backgroundColorId,
        textColorId,
        popUpMenuTextColor,
    };

    PresetComp(CutTheMoogAudioProcessor& proc, PresetManager& manager);
    ~PresetComp();

    void paint (Graphics& g) override;
    void resized() override;
    void presetUpdated() override;

private:

    void loadPresetChoice();
    void addPresetOptions();
    void saveUserPreset();

    CutTheMoogAudioProcessor& proc;
    PresetManager& manager;

    ComboBox presetBox;
    TextEditor presetNameEditor;

    DrawableButton presetsLeft, presetsRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComp)

};

class PresetComponentItem : public foleys::GuiItem
{

public:

    FOLEYS_DECLARE_GUI_FACTORY(PresetComponentItem)

    PresetComponentItem (foleys::MagicGUIBuilder& builder, const ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({ { "presets-background", PresetComp::backgroundColorId },
                                { "presets-text", PresetComp::textColorId },
                                      {"menu-text", PresetComp::popUpMenuTextColor}});

        if (auto* proc = dynamic_cast<CutTheMoogAudioProcessor*> (builder.getMagicState().getProcessor()))
        {
            presetComp = std::make_unique<PresetComp> (*proc, proc->getPresetManager());
            addAndMakeVisible (presetComp.get());
        }
    }

    void update() override {}

    Component* getWrappedComponent() override
    {
        return presetComp.get();
    }

private:

    std::unique_ptr<PresetComp> presetComp;

};

