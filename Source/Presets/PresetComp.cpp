#include "PresetComp.h"


PresetComp::PresetComp(CutTheMoogAudioProcessor &proc, PresetManager &manager): proc (proc), manager (manager),
                                                                                presetsLeft("", DrawableButton::ImageOnButtonBackground),
                                                                                presetsRight("", DrawableButton::ImageOnButtonBackground) {

    manager.addListener(this);

    presetBox.setName("Preset Manager");
    //presetBox.setTooltip("Ehi, do you want to change some presets?");

    setColour (backgroundColorId, Colour (0xFF595C6B));
    setColour(textColorId, Colours::white);
    setColour(popUpMenuTextColor, Colours::white);

    addAndMakeVisible(presetBox);
    presetBox.setColour(ComboBox::backgroundColourId, Colours::transparentWhite);
    presetBox.setJustificationType(Justification::centred);
    presetBox.setTextWhenNothingSelected("No preset selected...");
    loadPresetChoice();

    addChildComponent(presetNameEditor);
    presetNameEditor.setColour (TextEditor::backgroundColourId, Colour (0xFF595C6B));
    presetNameEditor.setColour (TextEditor::textColourId, Colours::white);
    presetNameEditor.setColour (TextEditor::highlightColourId, Colour (0xFF8B3232));
    presetNameEditor.setColour (CaretComponent::caretColourId, Colour (0xFF8B3232));

    presetNameEditor.setFont (Font (16.0f).boldened());
    presetNameEditor.setMultiLine (false, false);
    presetNameEditor.setJustification (Justification::centred);

    auto setupButton = [=, &manager] (DrawableButton& button, Drawable* image, int presetOffset) {
        addAndMakeVisible(button);
        button.setImages(image, image, image);
        button.setWantsKeyboardFocus(false);
        button.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
        button.setColour (TextButton::buttonColourId, Colours::transparentBlack);
        button.onClick = [=, &manager] {
            auto idx = presetBox.getSelectedId() + presetOffset;
            while (idx <= 0)
                idx += manager.getNumPresets();
            while (idx > manager.getNumPresets())
                idx -= manager.getNumPresets();

            presetNameEditor.setVisible (false);
            presetBox.setSelectedId (idx, sendNotification);
        };
    };

    std::unique_ptr<Drawable> leftImage(Drawable::createFromImageData(BinaryData::LeftArrow_svg, BinaryData::LeftArrow_svgSize));
    std::unique_ptr<Drawable> rightImage(Drawable::createFromImageData(BinaryData::RightArrow_svg, BinaryData::RightArrow_svgSize));
    leftImage ->replaceColour(Colours::white, Colour(0xFF30CDE9));
    rightImage ->replaceColour(Colours::white, Colour(0xFF30CDE9));

    setupButton (presetsLeft, leftImage.get(), -1);
    setupButton(presetsRight, rightImage.get(), 1);

    presetUpdated();
    presetBox.onChange = [=, &proc] {
        const auto selectedId = presetBox.getSelectedId();
        if (selectedId >= 1000 || selectedId <=0)
            return;

        proc.setCurrentProgram(presetBox.getSelectedId() - 1);
    };
}

PresetComp::~PresetComp() {
    manager.removeListener(this);
}

void PresetComp::loadPresetChoice() {

    presetBox.getRootMenu() -> clear();

    const auto& presetChices = manager.getPresetChoices();
    std::map<String, PopupMenu> presetChoiceMap;

    for (int i = 0; i < presetChices.size(); ++i) {

        const String& choice = presetChices[i];
        String category = choice.upToFirstOccurrenceOf("_", false, false);
        if (category == "User")
            continue;

        category = (category == choice ) ? "KarotaPresets" : category;
        String presetName = choice.fromLastOccurrenceOf("_", false, false);

        if (presetChoiceMap.find(category) == presetChoiceMap.end())
            presetChoiceMap[category] = PopupMenu();

        presetChoiceMap[category].addItem(i + 1, presetName);
    }

    for (auto& presetGroup : presetChoiceMap) {
        presetBox.getRootMenu() ->addSubMenu(presetGroup.first, presetGroup.second);
    }

    auto& userPresetMenu = manager.getUserPresetMenu();
    if (userPresetMenu.containsAnyActiveItems())
        presetBox.getRootMenu() -> addSubMenu("User", userPresetMenu);

    addPresetOptions();
}

void PresetComp::addPresetOptions() {

    auto menu = presetBox.getRootMenu();
    menu -> addSeparator();

    PopupMenu::Item saveItem {"Save"};
    saveItem.itemID = 1001;
    saveItem.action = [=] { saveUserPreset(); };
    menu -> addItem(saveItem);

#if ! JUCE_IOS
    PopupMenu::Item goToFolderItem { "Go to Preset folder..." };
    goToFolderItem.itemID = 1002;
    goToFolderItem.action = [=] {
        presetUpdated();
        auto folder = manager.getUserPresetFolder();
        if (folder.isDirectory())
            folder.startAsProcess();
        else
            manager.chooseUserPresetFolder();
    };
    menu->addItem (goToFolderItem);

    PopupMenu::Item chooseFolderItem { "Choose Preset folder..." };
    chooseFolderItem.itemID = 1003;
    chooseFolderItem.action = [=] { presetUpdated(); manager.chooseUserPresetFolder(); };
    menu->addItem (chooseFolderItem);
#endif
}

void PresetComp::paint(Graphics &g) {

    constexpr auto cornerSize = 5.0f;
    presetBox.setColour (PopupMenu::ColourIds::backgroundColourId, findColour(backgroundColorId));
    presetNameEditor.setColour(TextEditor::textColourId, findColour(textColorId));
    presetBox.setColour(ComboBox::textColourId, findColour(textColorId));
    presetBox.setColour(PopupMenu::textColourId, findColour(popUpMenuTextColor));
    presetBox.setColour(PopupMenu::highlightedTextColourId, findColour(popUpMenuTextColor));
    g.setColour(findColour(backgroundColorId));

    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(22.0f, 0.0f), cornerSize);
}

void PresetComp::resized() {

    auto b = getLocalBounds();
    presetsLeft.setBounds(b.removeFromLeft(20));
    presetsRight.setBounds(b.removeFromRight(20));

    Rectangle<int> presetBound {22, 0, getWidth() - 44, getHeight() };
    presetBox.setBounds(presetBound);
    presetNameEditor.setBounds(presetBound);
    repaint();
}

void PresetComp::presetUpdated() {

    presetBox.setSelectedId(proc.getCurrentProgram() + 1, dontSendNotification);
}

void PresetComp::saveUserPreset() {

    presetNameEditor.setVisible(true);
    presetNameEditor.toFront(true);
    presetNameEditor.setText("MyPreset");
    presetNameEditor.setHighlightedRegion({0, 10});

    presetNameEditor.onReturnKey = [=] {
        auto presetName = presetNameEditor.getText();
        presetNameEditor.setVisible(false);

        if (manager.saveUserPreset(presetName, proc.getVts())){
            loadPresetChoice();
            proc.setCurrentProgram(manager.getNumPresets() - 1);
        } else{
            presetUpdated();
        }
    };
}