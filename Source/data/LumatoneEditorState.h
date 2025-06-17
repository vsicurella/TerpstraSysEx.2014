 /*
  ==============================================================================

    LumatoneEditorState.h
    Created: 29 Dec 2023 5:39pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_EDITOR_STATE_H
#define LUMATONE_EDITOR_STATE_H

#include "../lumatone_editor_library/data/application_state.h"
#include "../lumatone_editor_library/palettes/colour_palette_file.h"

#include "./LumatoneEditSelectionState.h"
#include "./LumatoneEditorBatchColourState.h"

#define CHOOSE_FILE_NOOP [](bool) -> void {}

struct LumatoneEditorFontLibrary;
class LumatoneEditorLookAndFeel;
class LumatoneController;
class LumatoneEditorColourPalette;

class ColourSelectionBroadcaster;
class ColourSelectionListener;
class ColourSelectionGroup;

namespace LumatoneEditorProperty
{
    static const juce::Identifier StateTree = juce::Identifier("LumatoneEditorState");

    static const juce::Identifier HasChangesToSave = juce::Identifier("HasChangesToSave");
    static const juce::Identifier HasChangesToSend = juce::Identifier("HasChangesToSend");

    static const juce::Identifier InCalibrationMode = juce::Identifier("InCalibrationMode");
    static const juce::Identifier FirmwareUpdatePerformed = juce::Identifier("FirmwareUpdatePerformed");

    static const juce::Identifier ColourPalettes = juce::Identifier("ColourPalettes");

    static const juce::Identifier CurrentFile = juce::Identifier("CurrentFile");
    static const juce::Identifier RecentFiles = juce::Identifier("RecentFiles");

    static const juce::Identifier AutoConnectDevice = juce::Identifier("AutoConnectDevice");
    static const juce::Identifier CheckDeviceActivity = juce::Identifier("CheckDeviceActivity");

    static const juce::Identifier UserDocumentsDirectory = juce::Identifier("UserDocumentsDirectory");
    static const juce::Identifier UserMappingsDirectory = juce::Identifier("UserMappingsDirectory");
    static const juce::Identifier UserPalettesDirectory = juce::Identifier("UserPalettesDirectory");

    static const juce::Identifier DeveloperModeOn = juce::Identifier("DeveloperModeOn");

    static const juce::Identifier MainWindowState = juce::Identifier("MainWindowState"); // For save/recall
    static const juce::Identifier MainWindowBounds = juce::Identifier("MainWindowBounds"); // For app drawing

    static const juce::Identifier EditorMode = juce::Identifier("EditorMode");

    // static const juce::Identifier SingleNoteNoteSetActive = juce::Identifier("SingleNoteNoteSetActive");
    // static const juce::Identifier SingleNoteChannelSetActive = juce::Identifier("SingleNoteChannelSetActive");
    // static const juce::Identifier SingleNoteColourSetActive = juce::Identifier("SingleNoteColourSetActive");
    // static const juce::Identifier SingleNoteKeyTypeSetActive = juce::Identifier("SingleNoteKeyTypeSetActive");
    // static const juce::Identifier SingleNoteAutoIncNoteActive = juce::Identifier("SingleNoteAutoIncNoteActive");
    // static const juce::Identifier SingleNoteAutoIncChannelActive = juce::Identifier("SingleNoteAutoIncChannelActive");
    // static const juce::Identifier SingleNoteAutoIncChannelAfterNumNotes = juce::Identifier("SingleNoteAutoIncChannelAfterNumNotes");
    // static const juce::Identifier SingleNoteCCFaderIsDefault = juce::Identifier("SingleNoteCCFaderIsDefault");

    // static const juce::Identifier IsomorphicMassAssign = juce::Identifier("IsomorphicMassAssign");

    static const juce::Identifier LastSettingsPanel = juce::Identifier("LastSettingsPanel");
    static const juce::Identifier LastColourWindowTab = juce::Identifier("LastColourWindowTab");
    static const juce::Identifier LastFirmwareBinPath = juce::Identifier("LastFirmwareBinPath");

    static const juce::Identifier ShowKeyProperties = juce::Identifier("ShowKeyProperties");
}

enum class EditorMode
{
    OFFLINE,
    ONLINE
};

namespace LumatoneEditor // TODO also use for EditorMode
{
    enum class MouseMode
    {
        SELECT,
        ASSIGN
    };
}

static juce::Array<juce::Identifier> GetLumatoneEditorProperties();

class LumatoneEditorState : public LumatoneApplicationState
{
public:
    // Only for "top level" state instance
    LumatoneEditorState(juce::ValueTree stateIn, LumatoneFirmwareDriver& driverIn, juce::UndoManager* undoManagerIn);

    LumatoneEditorState(juce::String name, const LumatoneEditorState& stateIn);
    LumatoneEditorState(const LumatoneEditorState& stateIn);

    virtual ~LumatoneEditorState() override;

    const juce::String getApplicationName() const { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() const { return ProjectInfo::versionString; }

    juce::Rectangle<int> getWindowBounds() const { return windowBounds; }
    float getRoundedRectCornerSize() const;

    bool getHasChangesToSave() const { return hasChangesToSave; }
    bool getInCalibrationMode() const { return inCalibrationMode; }
    bool firmwareUpdateCompleted() const { return firmwareUpdateWasPerformed; }
    bool getInDeveloperMode() const { return inDeveloperMode; }

    EditorMode getEditorMode() const { return editorMode; }
    bool inOfflineMode() const { return editorMode == EditorMode::OFFLINE; }
    bool inOnlineMode() const { return editorMode == EditorMode::ONLINE; }

    LumatoneEditor::MouseMode getMouseMode() const { return mouseMode; }
    bool inSelectMode() const { return mouseMode == LumatoneEditor::MouseMode::SELECT; }
    bool inAssignMode() const { return mouseMode == LumatoneEditor::MouseMode::ASSIGN; }

    // juce::Array<LumatoneKey> getSelectedKeys() const;

    LumatoneKeyPropertyData getEditSelectionData() const;

    LumatoneEditor::BatchColourEditData getBatchColourEditData() const;

    LumatoneEditorLookAndFeel& getEditorLookAndFeel() { return *lookAndFeel; }

    virtual const juce::Array<LumatoneEditorColourPalette>& getColourPalettes();

    const LumatoneEditorFontLibrary& getAppFonts() const { return *appFonts; }

    juce::String getProperty(juce::Identifier propertyId, juce::String fallbackValue=juce::String()) const;

    juce::File getCurrentFile() const { return currentFile; }

    juce::RecentlyOpenedFilesList& getRecentFiles();

    juce::File getUserDocumentsDirectory() const;
    juce::File getUserMappingsDirectory() const;
    juce::File getUserPalettesDirectory() const;

    juce::File getLastOpenedMappingsDirectory() const;


private:
    std::shared_ptr<ColourSelectionGroup> colourSelectionGroup;
public:
    void addColourSelectionBroadcaster(ColourSelectionBroadcaster* broadcasterIn);
    void removeColourSelectionBroadcaster(ColourSelectionBroadcaster* broadcasterIn);

    void addColourSelectionListener(ColourSelectionListener* listenerIn);
    void removeColourSelectionListener(ColourSelectionListener* listenerIn);

public:
    bool doSendChangesToDevice() const override;

protected:
    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    void loadPropertiesFile(juce::PropertiesFile* properties);

protected:
    bool hasChangesToSave = false;
    bool hasChangesToSend = false;
	bool inCalibrationMode = false;
    bool inDeveloperMode = false;
	bool firmwareUpdateWasPerformed = false;

    EditorMode editorMode = EditorMode::OFFLINE;
    LumatoneEditor::MouseMode mouseMode = LumatoneEditor::MouseMode::SELECT;

    LumatoneEditSelectionState editSelectionState;
    LumatoneEditorBatchColourState batchColourState;

private:
    std::shared_ptr<LumatoneEditorFontLibrary>      appFonts;
	std::shared_ptr<LumatoneEditorLookAndFeel>      lookAndFeel;
	std::shared_ptr<juce::Array<LumatoneEditorColourPalette>>        colourPalettes;

	juce::File currentFile;
	std::shared_ptr<juce::RecentlyOpenedFilesList>	recentFiles;

    juce::Rectangle<int> windowBounds;

    std::shared_ptr<juce::PropertiesFile>   propertiesFile;

    std::shared_ptr<LumatoneLayout>         lastSavedLayout;

protected:

    // Global UI constants

    const float contentBorderMarginWidthWindowH     = 0.02f;    // "padding" around content sections
    const float contentMarginWidthWindowH           = 0.013f;    // margin within content sections

    const float contentLabelHeightWindowH   = 0.033f;
    const float contentLabelFontScalar      = 0.5f;

    const float controlContentSmallMarginHeightWindowH  = 1.3e-2f;

    const float controlContentHeightParentH = 0.81f;
    const float controlLabelFontScalar      = 0.6f;

    const float controlHeightWindowH        = 2.38e-2f;


//================================================================================
public:
    class Controller : public LumatoneApplicationState::Controller
    {
    public:
        Controller(LumatoneEditorState& stateIn)
            : LumatoneApplicationState::Controller(stateIn)
            , editorState(stateIn) {}

        bool performAction(LumatoneAction* action, bool undoable=true, bool newTransaction=true) override;

        bool resetToCurrentFile();
        bool openRecentFile(int recentFileIndex);

        void addPalette(const LumatoneEditorColourPalette& newPalette);
        bool deletePaletteFile(juce::File pathToPalette);

        void setColourPalettes(const juce::Array<LumatoneEditorColourPalette>& palettesIn);
        void loadColourPalettesFromFile();

        bool setCurrentFile(juce::File fileToOpen, bool loadFile=true);
        bool saveMappingToFile(juce::File fileToSave);

        juce::PropertiesFile* getPropertiesFile() const { return editorState.propertiesFile.get(); }
        bool savePropertiesFile() const;

        // Sets a property in the juce::PropertyFile and saves it to file
        void savePropertyBoolValue(const juce::Identifier& id, bool value);
        void savePropertyIntValue(const  juce::Identifier& id, int value);
        void savePropertyStringValue(const  juce::Identifier& id, juce::String value);

        void setHasChangesToSave(bool hasChanges);
        void setCalibrationMode(bool calibrationModeOn);
        void setDeveloperMode(bool developerModeOn);
        void setEditMode(EditorMode editMode);

        void setAssignKeyColour(bool set, juce::Colour colourIn);
        void setAssignKeyType(bool set, LumatoneKeyType typeIn);
        void setAssignKeyNote(bool set, int noteIn);
        void setAssignKeyChannel(bool set, int channelIn);
        void setAssignCCFader(bool set, bool ccFaderDefaultIn);

        void setBatchColourBrightness(float value);
        void setBatchColourHueShift(float value);
        void setBatchColourTempShift(float value);

        void setWindowState(const juce::Rectangle<int>& windowBounds, juce::String stateString);

        void toggleKeyProperties();

    private:
        LumatoneEditorState& editorState;
    };

private:
    friend class LumatoneEditorStateController;
};


#endif // LUMATONE_EDITOR_STATE_H
