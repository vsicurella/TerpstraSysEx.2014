/*
  ==============================================================================

    This file was auto-generated!
    Created: XXX.2014
    Author:  hsstraub

  ==============================================================================
*/

#pragma once

#include "../data/LumatoneEditorState.h"

#include "../lumatone_editor_library/data/lumatone_layout.h"
#include "../lumatone_editor_library/listeners/status_listener.h"
#include "../lumatone_editor_library/listeners/editor_listener.h"
#include "../lumatone_editor_library/lumatone_midi_driver/firmware_types.h"

class LumatoneEditorKeyboardComponent;

class MidiEditArea;
class KeyEditorPanel;
class MappingSettingsPanel;
class GlobalSettingsArea;
class MappingTabBar;
enum class MappingEditorTabs;


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent : public juce::Component
                           , public LumatoneEditorState
                           , public LumatoneEditorState::Controller
                           , public LumatoneEditor::StatusListener
                           , public LumatoneEditor::EditorListener
                           , public juce::ChangeListener
                           , public juce::Button::Listener
{
public:
    //==============================================================================
    MainContentComponent(const LumatoneEditorState& stateIn, juce::ApplicationCommandManager* commandManager);
    ~MainContentComponent();

    void saveStateToPropertiesFile(PropertiesFile* propertiesFile);

    juce::TabbedButtonBar* getOctaveBoardSelectorTab();
    // CurvesArea* getCurvesArea() { return curvesArea.get(); }

    // Board edit operations
    LumatoneAction* createDeleteCurrentSectionAction(){return nullptr;};
    bool copyCurrentSubBoardData(){return false;};
    LumatoneAction* createPasteCurrentSectionAction(){return nullptr;};
    LumatoneAction* createModifiedPasteCurrentSectionAction(CommandID){return nullptr;};
    bool canPasteCopiedSubBoard() const;

    // Implementation of ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    // Implementation of Button::Listener
    void buttonClicked(juce::Button* btn) override;

    // GUI implementation
    void paint (juce::Graphics&) override;
    void resized() override;

    void resizeEditSectionTabs();
    void setMappingEditorTab(MappingEditorTabs tabIndex);

    // Implementation of LumatoneEditor::StatusListener
    void connectionStateChanged(ConnectionState state) override;

    // Implementation of LumatoneEditor::EditorListener
    void layoutImported(const LumatoneLayout& mappingData) override;

    //==============================================================================
    // Implementation of LumatoneEditorState

    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

private:

    void updateDeveloperMode();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)

    //==============================================================================
    // GUI components

    // Midi devices and connection state
    std::unique_ptr<MidiEditArea>			midiEditArea;

    // Sets of 55/56 keys
    std::unique_ptr<LumatoneEditorKeyboardComponent> 	allKeysOverview;

    // Edit fields for setting key and button parameters, and edits for single keys
    std::unique_ptr<KeyEditorPanel>			keyEditorPanel;
    std::unique_ptr<MappingSettingsPanel>	mappingSettingsComponent;

    std::unique_ptr<GlobalSettingsArea>		globalSettingsArea;

    std::unique_ptr<juce::Label>			lblSelectedKeys;

    std::unique_ptr<juce::Label> 			lblEditTitle;
    std::unique_ptr<MappingTabBar> 			sectionTabBar;
    juce::Component*						editorTabComponent = nullptr;

    std::unique_ptr<juce::TextButton>		btnLoadFile;
    std::unique_ptr<juce::TextButton>		btnSaveFile;
    std::unique_ptr<juce::TextButton>		btnImportFile;

    std::unique_ptr<juce::TextButton>		btnClearSelection;

    // Version signature in bottom left of window
    std::unique_ptr<juce::Label> 			lblAppName;
    std::unique_ptr<juce::Label> 			lblAppVersion;

    std::unique_ptr<juce::Label>			lblFirmwareVersion;

    // Buffer for copy/paste of sub board data
    std::unique_ptr<LumatoneBoard>			copiedSubBoardData;

    // Keyboard view controls
    std::unique_ptr<juce::TextButton>   toggleKeyPropertiesButton;


    //==============================================================================
    // Position and Size helpers

    int contentMargin;
    int contentWidth;

    int controlsLabelYPos;
    juce::Rectangle<int> controlsArea;
    juce::Rectangle<int> controlsAreaBackground;

    const float sectionTabsMarginW			= 0.013f;

    const float contentWidthRatio 			= 0.871f;

    const float headerHeight                = 0.0837f;

    const float controlSectionTabsY			= 0.63f;
    const float controlsLabelHeight 		= 0.033f;

    const float controlsAreaY               = 0.662f;
    const float controlsAreaHeight          = 0.43f;

    const float assignMarginX               = 0.043f;
    const float assignWidth                 = 0.5385f;
    const float assignHeight                = 0.44f;

    const float footerAreaY                 = 0.96f;

    const float lumatoneGraphicMarginTop	= 0.1f;
    const float lumatoneGraphicH			= 0.8f;

    const float fileButtonH					= 0.025f;
    const float importW						  = 0.06f;

    const float btnYFromImageTop			= 0.055f;
    const float saveLoadW					    = 0.055f;
    const float saveLoadMarginW				= 0.0034f;

    const float popupWidth                  = 0.4f;
    const float popupHeight                 = 0.333f;

    const float lumatoneVersionMarginX      = 0.02f;
    const float lumatoneVersionWidth        = 0.2f;
    const float lumatoneVersionHeight       = 0.5f;

    const float presetButtonColoursX        = 0.7237f;
    const float presetButtonColoursHeight   = 0.2258f;

    const float settingsColumnX             = 0.597826f;
    const float settingsAreaY               = 0.546875f;
    const float settingsAreaHeight          = 0.148148f;

    const juce::Rectangle<float> generalSettingsBounds = { settingsColumnX, settingsAreaY, 0.17f, settingsAreaHeight };
    const juce::Rectangle<float>   pedalSettingsBounds = { 0.777778f,       settingsAreaY, 0.18f, settingsAreaHeight };
    const juce::Rectangle<float>      curvesAreaBounds = { settingsColumnX, 0.7174f,       0.3626f, 0.21f };

};
