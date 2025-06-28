/*
==============================================================================

    LumatoneEditorState.cpp
    Created: 29 Dec 2023 5:39pm
    Author:  Vincenzo

==============================================================================
*/

#include "LumatoneEditorState.h"

#include "../style/LumatoneEditorLookAndFeel.h"

#include "../lumatone_editor_library/device/lumatone_controller.h"
#include "../lumatone_editor_library/listeners/editor_listener.h"
#include "../lumatone_editor_library/palettes/colour_selection_group.h"

#include "../actions/EditorControlActions.h"
#include "../actions/KeySelectionControlActions.h"

static juce::File getDefaultUserDocumentsDirectory()
{
    return File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Lumatone Editor");
}
static juce::File getDefaultUserMappingsDirectory()
{
    return getDefaultUserDocumentsDirectory().getChildFile("Mappings");
}
static juce::File getDefaultUserPalettesDirectory()
{
    return getDefaultUserDocumentsDirectory().getChildFile("Palettes");
}

juce::Array<juce::Identifier> GetLumatoneEditorProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneEditorProperty::HasChangesToSave);
    properties.add(LumatoneEditorProperty::HasChangesToSend);
    properties.add(LumatoneEditorProperty::InCalibrationMode);
    properties.add(LumatoneEditorProperty::FirmwareUpdatePerformed);
    properties.add(LumatoneEditorProperty::ColourPalettes);
    properties.add(LumatoneEditorProperty::CurrentFile);
    properties.add(LumatoneEditorProperty::RecentFiles);
    properties.add(LumatoneEditorProperty::UserDocumentsDirectory);
    properties.add(LumatoneEditorProperty::UserMappingsDirectory);
    properties.add(LumatoneEditorProperty::UserPalettesDirectory);
    properties.add(LumatoneEditorProperty::DeveloperModeOn);
    properties.add(LumatoneEditorProperty::EditorMode);
    return properties;
}

LumatoneEditorState::LumatoneEditorState(juce::ValueTree stateIn, LumatoneFirmwareDriver &driverIn, juce::UndoManager *undoManagerIn)
    : LumatoneApplicationState(stateIn, driverIn, undoManagerIn)
    , editSelectionState(name + "_EditSelection", state)
    , batchColourState(name + "_BatchColourState", state)
{
    // DBG(name + " LumatoneEditorState created");
    appFonts = std::make_shared<LumatoneEditorFontLibrary>();
    lookAndFeel = std::make_shared<LumatoneEditorLookAndFeel>(*appFonts, true);

    loadPropertiesFile(nullptr);

    lastSavedLayout = std::make_shared<LumatoneLayout>();

    colourPalettes = std::make_shared<juce::Array<LumatoneEditorColourPalette>>();
    colourSelectionGroup = std::make_shared<ColourSelectionGroup>("LumatoneEditorAppColourGroup");
}

LumatoneEditorState::LumatoneEditorState(juce::String name, const LumatoneEditorState &stateIn)
    : LumatoneApplicationState(name, stateIn)
    , editSelectionState(name, state)
    , batchColourState(name, state)
    , appFonts(stateIn.appFonts)
    , lookAndFeel(stateIn.lookAndFeel)
    , recentFiles(stateIn.recentFiles)
    , propertiesFile(stateIn.propertiesFile)
    , lastSavedLayout(stateIn.lastSavedLayout)
    , colourPalettes(stateIn.colourPalettes)
    , colourSelectionGroup(stateIn.colourSelectionGroup)
{
    // DBG(name + " LumatoneEditorState created");
    loadStateProperties(stateIn.state);
}

LumatoneEditorState::LumatoneEditorState(const LumatoneEditorState &stateIn)
    : LumatoneEditorState(stateIn.name + "Copy", stateIn)
{
    // DBG(name + " LumatoneEditorState created");
}

LumatoneEditorState::~LumatoneEditorState()
{
    if (name == LumatoneEditorProperty::StateTree.toString())
        DBG(state.toXmlString());

    colourSelectionGroup = nullptr;

    recentFiles = nullptr;
    propertiesFile = nullptr;
    lookAndFeel = nullptr;
    appFonts = nullptr;

    // DBG(name + " LumatoneEditorState deleted");
}

float LumatoneEditorState::getRoundedRectCornerSize() const
{
    return windowBounds.getHeight() * lookAndFeel->getRoundedRectCornerToAppHeightRatio();
}

juce::String LumatoneEditorState::getProperty(juce::Identifier propertyId, juce::String fallbackValue) const
{
    return propertiesFile->getValue(propertyId, fallbackValue);
}

juce::RecentlyOpenedFilesList& LumatoneEditorState::getRecentFiles()
{
    return *recentFiles;
}

LumatoneKeyPropertyData LumatoneEditorState::getEditSelectionData() const
{
    return editSelectionState.getData();
}

LumatoneEditor::BatchColourEditData LumatoneEditorState::getBatchColourEditData() const
{
    return batchColourState.getData();
}

const juce::Array<LumatoneEditorColourPalette> &LumatoneEditorState::getColourPalettes()
{
    return *colourPalettes;
}

juce::File LumatoneEditorState::getUserDocumentsDirectory() const
{
    juce::String possibleDirectory = propertiesFile->getValue(LumatoneEditorProperty::UserDocumentsDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || directory.existsAsFile())
    {
        directory = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Lumatone Editor");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneEditorState::getUserMappingsDirectory() const
{
    juce::File parentFolder = getUserDocumentsDirectory();

    juce::String possibleDirectory = propertiesFile->getValue(LumatoneEditorProperty::UserMappingsDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || parentFolder.isDirectory())
    {
        directory = parentFolder.getChildFile("Mappings");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneEditorState::getUserPalettesDirectory() const
{
    juce::File parentFolder = getUserDocumentsDirectory();

    juce::String possibleDirectory = propertiesFile->getValue(LumatoneEditorProperty::UserPalettesDirectory);
    juce::File directory;

    if (juce::File::isAbsolutePath(possibleDirectory))
    {
        directory = juce::File(possibleDirectory);
    }
    if (!directory.exists() || parentFolder.isDirectory())
    {
        directory = parentFolder.getChildFile("Palettes");

        // User pref?
        directory.createDirectory();
    }
    return directory;
}

juce::File LumatoneEditorState::getLastOpenedMappingsDirectory() const
{
    return recentFiles->getFile(0).getParentDirectory();
}

void LumatoneEditorState::addColourSelectionBroadcaster(ColourSelectionBroadcaster *broadcasterIn)
{
    colourSelectionGroup->addSelector(broadcasterIn);
}

void LumatoneEditorState::removeColourSelectionBroadcaster(ColourSelectionBroadcaster *broadcasterIn)
{
    colourSelectionGroup->removeSelector(broadcasterIn);
}

void LumatoneEditorState::addColourSelectionListener(ColourSelectionListener *listenerIn)
{
    colourSelectionGroup->addColourSelectionListener(listenerIn);
}

void LumatoneEditorState::removeColourSelectionListener(ColourSelectionListener *listenerIn)
{
    colourSelectionGroup->removeColourSelectionListener(listenerIn);
}

bool LumatoneEditorState::doSendChangesToDevice() const
{
    return LumatoneApplicationState::doSendChangesToDevice() && editorMode == EditorMode::ONLINE;
}

void LumatoneEditorState::Controller::setEditMode(EditorMode editMode)
{
    editorState.editorMode = editMode;
    editorState.setStateProperty(LumatoneEditorProperty::EditorMode, (int)editorState.editorMode);
}

void LumatoneEditorState::Controller::setMouseMode(LumatoneEditor::MouseMode mouseMode)
{
    editorState.mouseMode = mouseMode;
    editorState.setStateProperty(LumatoneEditorProperty::MouseMode, (int)editorState.mouseMode);
}

void LumatoneEditorState::Controller::setAssignKeyColour(bool set, juce::Colour colourIn)
{
    editorState.editSelectionState.setKeyColour(set, colourIn);
}

void LumatoneEditorState::Controller::setAssignKeyType(bool set, LumatoneKeyType typeIn)
{
    editorState.editSelectionState.setKeyType(set, typeIn);
}

void LumatoneEditorState::Controller::setAssignKeyNote(bool set, int noteIn)
{
    editorState.editSelectionState.setKeyNote(set, noteIn);
}

void LumatoneEditorState::Controller::setAssignKeyChannel(bool set, int channelIn)
{
    editorState.editSelectionState.setKeyChannel(set, channelIn);
}

void LumatoneEditorState::Controller::setAssignCCFader(bool set, bool ccFaderDefaultIn)
{
    editorState.editSelectionState.setCCFader(set, ccFaderDefaultIn);
}

void LumatoneEditorState::Controller::setIncrementNotes(bool incrementNotes)
{
    editorState.incrementNotesMode = incrementNotes;
    editorState.setStateProperty(LumatoneEditorProperty::AutoIncNoteActive, incrementNotes);
}

void LumatoneEditorState::Controller::setChannelsIncrementPerNotes(int notesPerChannel)
{
    editorState.incrementChannelEvery = notesPerChannel;
    editorState.setStateProperty(LumatoneEditorProperty::AutoIncChannelAfterNumNotes, notesPerChannel);
}

void LumatoneEditorState::Controller::setBatchColourBrightness(float value)
{
    editorState.batchColourState.setBrightnessMultiplier(true, value);
}

void LumatoneEditorState::Controller::setBatchColourHueShift(float value)
{
    editorState.batchColourState.setHueShiftAmount(true, value);
}

void LumatoneEditorState::Controller::setBatchColourTempShift(float value)
{
    editorState.batchColourState.setTempShiftAmount(true, value);
}

void LumatoneEditorState::Controller::setWindowState(const juce::Rectangle<int> &windowBounds, juce::String stateString)
{
    editorState.windowBounds = windowBounds;
    editorState.setStateProperty(LumatoneEditorProperty::MainWindowState, stateString);
    editorState.setStateProperty(LumatoneEditorProperty::MainWindowBounds, windowBounds.toString());
}

void LumatoneEditorState::Controller::toggleKeyProperties()
{
    bool toggled = !editorState.getBoolProperty(LumatoneEditorProperty::ShowKeyProperties, false);
    editorState.setStateProperty(LumatoneEditorProperty::ShowKeyProperties, toggled);
}

bool LumatoneEditorState::Controller::DoEditKeyDownAction(LumatoneKeyCoord keyCoord, juce::ModifierKeys mods)
{
    bool keyIsSelected = editorState.isKeySelected(keyCoord);

    if (mods.isCommandDown())
    {
        if (editorState.mouseMode == LumatoneEditor::MouseMode::SELECT)
            return performAction(new AddOrRemoveKeySelectionAction(editorState, editorState.getMappingData()->keyCoordToKeyNum(keyCoord), keyIsSelected));
    }
    else
    {
        // Select mode allows toggling a selected key
        // But assign mode selects a key, updates it, but does not deselect on another click

        if (editorState.mouseMode == LumatoneEditor::MouseMode::SELECT)
        {
            juce::Array<MappedLumatoneKey> keySelection;
            if (!keyIsSelected)
            {
                keySelection.add(MappedLumatoneKey(editorState.getKey(keyCoord), keyCoord));
            }

            performAction(new SetKeySelectionAction(editorState, keySelection));
        }
        else if (!keyIsSelected)
        {
            juce::Array<MappedLumatoneKey> keySelection;
            keySelection.add(MappedLumatoneKey(editorState.getKey(keyCoord), keyCoord));
            performAction(new SetKeySelectionAction(editorState, keySelection));
        }

        if (editorState.mouseMode == LumatoneEditor::MouseMode::ASSIGN)
        {
            auto keySelection = editorState.getSelectedKeys()->getLast();
            jassert(keySelection.keyIndex >= 0);

            auto selectionData = editorState.editSelectionState.getData();

            if (editorState.incrementNotesMode)
            {
                // Increment note/channel
                int note = selectionData.note + 1;
                int channel = selectionData.channel;

                // Auto increment channel
                if (editorState.incrementChannelEvery > 0 && note >= editorState.incrementChannelEvery)
                {
                    note = 0;
                    channel++;
                    if (channel > 16)
                        channel = 1;
                }

                if (note > 127)
                    note = 0;

                selectionData.note = note;
                selectionData.channel = channel;
                performAction(new SetKeySettingsAction(editorState, selectionData), true, true);
            }

            // Assign new values - TODO might want to fine tune how the new actions are queued here
            bool assignIsNewAction = keyIsSelected && !editorState.incrementNotesMode;
            return performAction(new ApplyAssignmentsToSelectionAction(editorState, selectionData, keySelection), true, assignIsNewAction);
        }
    }

    return false;
}

juce::ValueTree LumatoneEditorState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneEditorProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneEditorProperty::StateTree);

    // LumatoneApplicationState::loadStateProperties(newState);

    // DBG("LumatoneApplicationState::loadStateProperties:\n" + newState.toXmlString());
    for (auto property : GetLumatoneEditorProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    return newState;
}

void LumatoneEditorState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    // DBG(name + " state changed");
    // DBG(state.toXmlString());

    LumatoneApplicationState::handleStatePropertyChange(stateIn, property);

    if (property == LumatoneEditorProperty::MainWindowBounds)
    {
        windowBounds = juce::Rectangle<int>::fromString(stateIn[property].toString());
    }
    else if (property == LumatoneEditorProperty::MouseMode)
    {
        mouseMode = LumatoneEditor::MouseMode((int)stateIn[property]);
    }
    else if (property == LumatoneEditorProperty::EditorMode)
    {
        editorMode = EditorMode((int)stateIn[property]);
    }
    else if (property == LumatoneEditorProperty::AutoIncNoteActive)
    {
        incrementNotesMode = (bool)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::AutoIncChannelAfterNumNotes)
    {
        incrementChannelEvery = (int)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::HasChangesToSend)
    {
        hasChangesToSend = (bool)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::InCalibrationMode)
    {
        inCalibrationMode =  (bool)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::FirmwareUpdatePerformed)
    {
        firmwareUpdateWasPerformed = (bool)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::CurrentFile)
    {
        currentFile = juce::File(stateIn[property]);
    }
    else if (property == LumatoneEditorProperty::DeveloperModeOn)
    {
        inDeveloperMode = (bool)stateIn[property];
    }
    else if (property == LumatoneEditorProperty::HasChangesToSave)
    {
        hasChangesToSave = (bool)stateIn[property];
    }
}

void LumatoneEditorState::loadPropertiesFile(juce::PropertiesFile *propertiesIn)
{
    juce::PropertiesFile::Options options;
    options.applicationName = "LumatoneSetup";
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
    options.folderName = "~/.config/LumatoneSetup";
#else
    options.folderName = "LumatoneSetup";
#endif

    propertiesFile = std::make_shared<juce::PropertiesFile>(options);
    DBG(propertiesFile->createXml("LumatoneEditorSettings")->toString());
    jassert(propertiesFile != nullptr);

    recentFiles = std::make_shared<juce::RecentlyOpenedFilesList>();
    recentFiles->restoreFromString(propertiesFile->getValue(LumatoneEditorProperty::RecentFiles));
    recentFiles->removeNonExistentFiles();

    LumatoneApplicationState::loadPropertiesFile(propertiesFile.get());

    setStateProperty(LumatoneEditorProperty::UserDocumentsDirectory, propertiesFile->getValue(LumatoneEditorProperty::UserDocumentsDirectory.toString(), getDefaultUserDocumentsDirectory().getFullPathName()));
    setStateProperty(LumatoneEditorProperty::UserMappingsDirectory, propertiesFile->getValue(LumatoneEditorProperty::UserMappingsDirectory.toString(), getDefaultUserMappingsDirectory().getFullPathName()));
    setStateProperty(LumatoneEditorProperty::UserPalettesDirectory, propertiesFile->getValue(LumatoneEditorProperty::UserPalettesDirectory.toString(), getDefaultUserPalettesDirectory().getFullPathName()));
    setStateProperty(LumatoneEditorProperty::RecentFiles, propertiesFile->getValue(LumatoneEditorProperty::RecentFiles.toString(), juce::String()));
    setStateProperty(LumatoneEditorProperty::MainWindowState, propertiesFile->getValue(LumatoneEditorProperty::MainWindowState.toString(), juce::String()));

    setStateProperty(LumatoneEditorProperty::AutoConnectDevice, propertiesFile->getBoolValue(LumatoneEditorProperty::AutoConnectDevice.toString(), true));

    // setStateProperty(LumatoneEditorProperty::SingleNoteKeyTypeSetActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteKeyTypeSetActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteNoteSetActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteNoteSetActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteChannelSetActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteChannelSetActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteColourSetActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteColourSetActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteCCFaderIsDefault, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteCCFaderIsDefault.toString(), false));
    // setStateProperty(LumatoneEditorProperty::SingleNoteAutoIncNoteActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteAutoIncNoteActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteAutoIncChannelActive, propertiesFile->getBoolValue(LumatoneEditorProperty::SingleNoteAutoIncChannelActive.toString(), true));
    // setStateProperty(LumatoneEditorProperty::SingleNoteAutoIncChannelAfterNumNotes, propertiesFile->getIntValue(LumatoneEditorProperty::SingleNoteAutoIncChannelAfterNumNotes.toString(), 127));

    // setStateProperty(LumatoneEditorProperty::IsomorphicMassAssign, propertiesFile->getBoolValue(LumatoneEditorProperty::IsomorphicMassAssign.toString(), false))

    setStateProperty(LumatoneEditorProperty::LastSettingsPanel, propertiesFile->getIntValue(LumatoneEditorProperty::LastSettingsPanel.toString(), 1));
    setStateProperty(LumatoneEditorProperty::LastColourWindowTab, propertiesFile->getIntValue(LumatoneEditorProperty::LastColourWindowTab.toString(), 1));
    setStateProperty(LumatoneEditorProperty::LastFirmwareBinPath, propertiesFile->getValue(LumatoneEditorProperty::LastFirmwareBinPath.toString()
        , juce::File::getSpecialLocation(juce::File::SpecialLocationType::userHomeDirectory).getFullPathName()
        ));

    setStateProperty(LumatoneEditorProperty::DeveloperModeOn, propertiesFile->getBoolValue(LumatoneEditorProperty::DeveloperModeOn, false));
}

void LumatoneEditorState::Controller::setColourPalettes(const juce::Array<LumatoneEditorColourPalette> &palettesIn)
{
    *editorState.colourPalettes = palettesIn;
    // TODO
    // editorState.setPropertyExcludingListener(this, LumatoneEditorProperty::ColourPalettes, "", nullptr);
}

void LumatoneEditorState::Controller::loadColourPalettesFromFile()
{
    auto directory = editorState.getUserPalettesDirectory();
    auto foundPaletteFiles = directory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, '*' + juce::String(PALETTEFILEEXTENSION));

    juce::Array<LumatoneEditorColourPalette> newPalettes;

    auto paletteSorter = LumatoneEditorPaletteSorter();
    for (auto file : foundPaletteFiles)
    {
        LumatoneEditorColourPalette palette = LumatoneEditorColourPalette::loadFromFile(file);
        newPalettes.addSorted(paletteSorter, palette);
    }

    setColourPalettes(newPalettes);
}

void LumatoneEditorState::Controller::addPalette(const LumatoneEditorColourPalette &newPalette)
{
    editorState.colourPalettes->add(newPalette);
    // TODO
    // editorState.setPropertyExcludingListener(this, LumatoneEditorProperty::ColourPalettes, "", nullptr);
}

bool LumatoneEditorState::Controller::deletePaletteFile(juce::File pathToPalette)
{
	bool success = false;

	if (pathToPalette.existsAsFile())
	{
		success = pathToPalette.deleteFile();
	}

	return success;
}

bool LumatoneEditorState::Controller::performAction(LumatoneAction *action, bool undoable, bool newTransaction)
{
    if (LumatoneApplicationState::Controller::performAction(action, undoable, newTransaction))
    {

        return true;
    }

    return false;
}

// Open a SysEx mapping from the file specified in currentFile
bool LumatoneEditorState::Controller::resetToCurrentFile()
{
    if (editorState.getCurrentFile().getFullPathName().isEmpty())
    {
        // Replace with blank file
		LumatoneLayout defaultLayout;
        editorState.setCompleteConfig(defaultLayout);
        *editorState.lastSavedLayout = defaultLayout;
        setHasChangesToSave(false);
        return true;
    }

    if (editorState.getCurrentFile().existsAsFile())
	{
		// XXX StringArray format: platform-independent?
		juce::StringArray stringArray;
		editorState.getCurrentFile().readLines(stringArray);
		LumatoneLayout keyMapping;
        if (!keyMapping.fromStringArray(stringArray))
        {
	        AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Open File Error", "The file " + editorState.getCurrentFile().getFullPathName() + " could not be read properly.");
            return false;
        }

        // Mark file as unchanged (would prefer to  do this after, but this works better for callbacks)
        setHasChangesToSave(false);

		// Send configuration to controller, if connected
        editorState.setCompleteConfig(keyMapping);

        // Save copy that doesn't get updated
        *editorState.lastSavedLayout = keyMapping;

        // Clear undo history
		editorState.undoManager->clearUndoHistory();

		// Add file to recent files list
		editorState.recentFiles->addFile(editorState.currentFile);

        // getEditorListeners()->call(&LumatoneEditor::EditorListener::newFileLoaded, editorState.getCurrentFile());

		return true;
	}

	// Show error message
	AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Open File Error", "The file " + editorState.getCurrentFile().getFullPathName() + " could not be opened.");

	// XXX Update Window title in any case? Make file name empty/make data empty in case of error?
	return false;
}

bool LumatoneEditorState::Controller::setCurrentFile(File fileToOpen, bool loadFile)
{
    editorState.currentFile = fileToOpen;
    editorState.setStateProperty(LumatoneEditorProperty::CurrentFile, editorState.currentFile.getFullPathName());
    if (loadFile)
        return resetToCurrentFile();
    return true;
}

// open a file from the "recent files" menu
bool LumatoneEditorState::Controller::openRecentFile(int recentFileIndex)
{
    jassert(recentFileIndex >= 0 && recentFileIndex < editorState.recentFiles->getNumFiles());
    return setCurrentFile(editorState.recentFiles->getFile(recentFileIndex));
}

bool LumatoneEditorState::Controller::saveMappingToFile(juce::File fileToSave)
{
    juce::StringArray stringArray = editorState.getMappingData()->toStringArray();
    juce::String fileText = stringArray.joinIntoString("\n");

    bool success = false;

    if (fileToSave.existsAsFile())
        success = fileToSave.replaceWithText(fileText, false, false);

    else if (fileToSave.create().ok())
    {
        success = fileToSave.appendText(fileText, false, false);
    }

    if (success && editorState.getCurrentFile() != fileToSave)
    {
        // TODO skip certain updates?
        setCurrentFile(fileToSave);
    }

    return success;
}

bool LumatoneEditorState::Controller::savePropertiesFile() const
{
    // TODO Save documents directories (Future: provide option to change them and save after changed by user)
    //propertiesFile->setValue(LumatoneEditorProperty::UserDocumentsDirectory, getUserDocumentsDirectory().getFullPathName());
    //propertiesFile->setValue(LumatoneEditorProperty::UserMappingsDirectory, getUserMappingsDirectory().getFullPathName());
    //propertiesFile->setValue(LumatoneEditorProperty::UserPalettesDirectory, getUserPalettesDirectory().getFullPathName());

    // Save recent files list
    editorState.recentFiles->removeNonExistentFiles();
    jassert(editorState.propertiesFile != nullptr);
    editorState.propertiesFile->setValue(LumatoneEditorProperty::RecentFiles, editorState.recentFiles->toString());

    auto saveWindowState = editorState.getStringProperty(LumatoneEditorProperty::MainWindowState);
    editorState.propertiesFile->setValue(LumatoneEditorProperty::MainWindowState, saveWindowState);

    return editorState.propertiesFile->saveIfNeeded();
}

void LumatoneEditorState::Controller::savePropertyBoolValue(const juce::Identifier &id, bool value)
{
    editorState.propertiesFile->setValue(id.toString(), juce::var(value));
    savePropertiesFile();
    editorState.setStateProperty(id, juce::var(value));
}

void LumatoneEditorState::Controller::savePropertyIntValue(const juce::Identifier &id, int value)
{
    editorState.propertiesFile->setValue(id.toString(), juce::var(value));
    savePropertiesFile();
    editorState.setStateProperty(id, juce::var(value));
}

void LumatoneEditorState::Controller::savePropertyStringValue(const juce::Identifier &id, juce::String value)
{
    editorState.propertiesFile->setValue(id.toString(), juce::var(value));
    savePropertiesFile();
    editorState.setStateProperty(id, juce::var(value));
}

void LumatoneEditorState::Controller::setHasChangesToSave(bool hasChanges)
{
    editorState.hasChangesToSave = hasChanges;
    editorState.setStateProperty(LumatoneEditorProperty::HasChangesToSave, hasChanges);
}

void LumatoneEditorState::Controller::setCalibrationMode(bool calibrationModeOn)
{
    editorState.inCalibrationMode = calibrationModeOn;
    editorState.getLumatoneController()->setCalibratePitchModWheel(calibrationModeOn);
    editorState.setStateProperty(LumatoneEditorProperty::InCalibrationMode, editorState.inCalibrationMode);
    savePropertyBoolValue(LumatoneEditorProperty::InCalibrationMode, calibrationModeOn);
}

void LumatoneEditorState::Controller::setDeveloperMode(bool developerModeOn)
{
    editorState.inDeveloperMode = developerModeOn;
    editorState.setStateProperty(LumatoneEditorProperty::DeveloperModeOn, editorState.inDeveloperMode);
    savePropertyBoolValue(LumatoneEditorProperty::DeveloperModeOn, developerModeOn);
}
