#include "PresetManager.h"
#include "PresetComp.h"

namespace
{
    static String userPresetPath = "KarotaSound/CutTheMooog/UserPresets.txt";
}

Preset::Preset(String presetFile) {

    String xmlText;

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {

        if (String (BinaryData::originalFilenames[i]) == presetFile){
            int dummy = 0;
            xmlText = String (BinaryData::getNamedResource(BinaryData::namedResourceList[i], dummy));
        }

    }

    jassert(xmlText.isNotEmpty());
    initialise(ValueTree::fromXml(xmlText));

}

Preset::Preset(const File &presetFile) {

    String xmlText = presetFile.loadFileAsString();
    initialise(ValueTree::fromXml(xmlText));
}

void Preset::initialise(const ValueTree &parentTree) {

    name = parentTree.getProperty("name").toString();
    jassert(name.isNotEmpty());

    state = parentTree.getChildWithName("PARAMETERS");

    for (int i = 0; i < state.getNumChildren(); ++i) {

        auto child = state.getChild(i);
        if (child.getProperty("id").toString() == "preset") // the property preset under PARAMETERS is the preset number
            index = (int) child.getProperty("value");
    }
}

PresetManager::PresetManager() {

#if JUCE_IOS
    File appDataDir = File::getSpecialLocation (File::userApplicationDataDirectory);
    userPresetFolder = appDataDir.getChildFile (userPresetPath).getSiblingFile ("Presets");
    if (! userPresetFolder.isDirectory())
    {
        userPresetFolder.deleteFile();
        userPresetFolder.createDirectory();
    }
#endif

    loadPresets();

}

StringArray PresetManager::getPresetChoices() {

    StringArray choices;

    for (int i = 0; i < maxIdx; ++i) {

        if (presetMap[i] != nullptr)
            choices.add(presetMap[i] -> name);
    }

    return choices;
}

void PresetManager::loadPresets() {

    // load factory presets

    presets.add (std::make_unique<Preset> ("Default.ctmpreset"));
    presets.add(std::make_unique<Preset>("HighDist.ctmpreset"));
    presets.add(std::make_unique<Preset>("LowFreqInt.ctmpreset"));
    numFactoryPresets = presets.size();

    for (auto* p: presets) {
        jassert( ! presetMap.contains (p -> index));
        presetMap.set(p -> index, p);
        maxIdx = jmax(maxIdx, p -> index);
    }

    maxIdx++;

    updateUserPresets();

}

String PresetManager::getPresetName(int idx) {

    if (!isPositiveAndBelow(idx, presets.size() )){

        jassertfalse;
        return {};
    }

    return presetMap[idx] -> name;
}

bool PresetManager::setPreset(AudioProcessorValueTreeState &vts, int idx) {

    if (!isPositiveAndBelow(idx, presets.size() )){

        jassertfalse;
        return false;
    }

    auto newState = presetMap[idx] -> state.createCopy();
    vts.replaceState(newState);
    return true;
}

void PresetManager::registerPresetsComponent(foleys::MagicGUIBuilder & builder) {

    static Identifier presetsID {"presets"};
    // here I need to register the new GUI component for the foleys magic gui
    builder.registerFactory (presetsID, &PresetComponentItem::factory);
}

bool PresetManager::saveUserPreset(const String &name, const AudioProcessorValueTreeState &vts) {

    if (! userPresetFolder.isDirectory())
        chooseUserPresetFolder();

    if (! userPresetFolder.isDirectory())
        return false;

    File saveFile = userPresetFolder.getChildFile(name + ".ctmpreset");
    saveFile.deleteFile();
    auto result = saveFile.create();

    if (result.failed())
        return false;

    auto xmlState = vts.state.createXml();
    if (xmlState == nullptr)
        return false;

    auto presetXml = std::make_unique<XmlElement>("Preset");
    presetXml ->setAttribute("name", "User_" + name);
    auto xmlParameters = std::make_unique<XmlElement>("PARAMETERS");
    for (auto* p : xmlState -> getChildWithTagNameIterator("PARAM")) {
        if (p ->getAttributeValue(0) == "preset")
            p ->setAttribute("value", maxIdx);

        xmlParameters ->addChildElement(new XmlElement(*p));
    }

    presetXml -> addChildElement(xmlParameters.release());

    saveFile.replaceWithText(presetXml -> toString());
    updateUserPresets();
    return true;
}

File PresetManager::getUserPresetConfigFile() const {

    File updatePresetFile = File::getSpecialLocation(File::userApplicationDataDirectory);
    return updatePresetFile.getChildFile(userPresetPath);
}

void PresetManager::chooseUserPresetFolder() {

#if ! JUCE_IOS
    FileChooser chooser ("Choose preset folder");
    if (chooser.browseForDirectory())
    {
        auto result = chooser.getResult();
        auto config = getUserPresetConfigFile();
        config.deleteFile();
        config.create();
        config.replaceWithText (result.getFullPathName());
        updateUserPresets();
    }
#endif
}

void PresetManager::loadPresetFolder(PopupMenu &menu, File &directory) {

    Array<File> presetFiles;

    for (auto& userPreset : directory.findChildFiles(File::findFilesAndDirectories, false)) {

        if (userPreset.isDirectory()){

            auto relativePath = userPreset.getRelativePathFrom(userPresetFolder);
            auto firstSubFolder = relativePath.fromLastOccurrenceOf(File::getSeparatorString(), false, false);

            PopupMenu subMenu;
            loadPresetFolder(subMenu, userPreset);
            menu.addSubMenu(firstSubFolder, subMenu);
        }
        if (userPreset.hasFileExtension(".ctmpreset"))
            presetFiles.add(userPreset);
    }

    for (auto& userPreset : presetFiles) {
        auto relativePath = userPreset.getRelativePathFrom(userPresetFolder);
        auto newPreset = presets.add(std::make_unique<Preset>(userPreset));
        newPreset -> index = maxIdx;
        presetMap.set(newPreset -> index, newPreset);
        menu.addItem(newPreset -> index + 1, newPreset->name.fromFirstOccurrenceOf ("User_", false, false));
        maxIdx++;
    }
}

void PresetManager::updateUserPresets() {

#if ! JUCE_IOS
    // set preset folder
    auto config = getUserPresetConfigFile();
    if (config.existsAsFile())
        userPresetFolder = File (config.loadFileAsString());
    else
        userPresetFolder = File();
#endif

    // remove existing user presets
    presets.removeRange (numFactoryPresets, maxIdx - numFactoryPresets);
    for (; maxIdx > numFactoryPresets; maxIdx--)
        presetMap.remove (maxIdx - 1);
    userPresetMenu.clear();

    if (userPresetFolder.isDirectory())
        loadPresetFolder (userPresetMenu, userPresetFolder);
}
