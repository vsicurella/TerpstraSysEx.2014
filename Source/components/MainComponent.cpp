/*
  ==============================================================================

  MainComponent.cpp
  Created: XXX.2014
  Author:  hsstraub

  ==============================================================================
*/

#include "MainComponent.h"

#include "../style/LumatoneEditorLookAndFeel.h"

#include "../LumatoneMenu.h"

#include "./MidiEditArea.h"
#include "./GlobalSettingsArea.h"
#include "./palettes/ColourSelectorPanel.h"

#include "./mapping/KeyEditorPanel.h"
#include "./mapping/MappingSettingsPanel.h"
#include "./mapping/MappingTabBar.h"

#include "../lumatone_editor_library/palettes/colour_edit_textbox.h"
// #include "../lumatone_editor_library/ui/keyboard_component.h"
#include "./LumatoneEditorKeyboard.h"
#include "../lumatone_editor_library/graphics/view_constants.h"
#include "../lumatone_editor_library/actions/edit_actions.h"


//==============================================================================
MainContentComponent::MainContentComponent(const LumatoneEditorState& stateIn, juce::ApplicationCommandManager* commandManager)
    : LumatoneEditorState("MainComponent", stateIn)
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    , copiedSubBoardData(std::make_unique<LumatoneBoard>())
{
    setName("MainContentComponent");

    // Midi input + output
    midiEditArea.reset(new MidiEditArea(stateIn));
    addAndMakeVisible(midiEditArea.get());

    // All keys overview
    allKeysOverview.reset(new LumatoneEditorKeyboardComponent(stateIn));
    allKeysOverview->setUiMode(LumatoneKeyboardComponent::UiMode::Controller);
    addAndMakeVisible(allKeysOverview.get());

    // Edit function area
    keyEditorPanel = std::make_unique<KeyEditorPanel>(stateIn);
    addChildComponent(*keyEditorPanel);

    mappingSettingsComponent = std::make_unique<MappingSettingsPanel>(stateIn);
    addChildComponent(*mappingSettingsComponent);

    globalSettingsArea.reset(new GlobalSettingsArea(stateIn));
    addAndMakeVisible(globalSettingsArea.get());
    globalSettingsArea->listenToColourEditButtons(this);

    lblSelectedKeys.reset(new Label("lblSelectedKeys", "0 Keys Selected"));
    lblSelectedKeys->setFont(getAppFonts().getFont(LumatoneEditorFont::UniviaProBold));
    lblSelectedKeys->setJustificationType(juce::Justification::centredRight);
    lblSelectedKeys->setColour(Label::ColourIds::textColourId, getEditorLookAndFeel().findColour(LumatoneEditorColourIDs::NumKeySelectedText));
    lblSelectedKeys->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    addAndMakeVisible(lblSelectedKeys.get());

    lblEditTitle.reset(new Label("lblEditTitle", "Edit Mapping"));
    lblEditTitle->setFont(getAppFonts().getFont(LumatoneEditorFont::UniviaProBold));
    lblEditTitle->setColour(Label::ColourIds::textColourId, getEditorLookAndFeel().findColour(LumatoneEditorColourIDs::LabelBlue));
    lblEditTitle->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    addAndMakeVisible(lblEditTitle.get());

    sectionTabBar = std::make_unique<MappingTabBar>();
    sectionTabBar->addChangeListener(this);
    addAndMakeVisible(sectionTabBar.get());
    setMappingEditorTab(MappingEditorTabs::KeyEditor);

    btnLoadFile.reset(new juce::TextButton("btnLoadFile"));
    addAndMakeVisible(btnLoadFile.get());
    btnLoadFile->setButtonText(juce::translate("LoadFile"));
    btnLoadFile->setCommandToTrigger(commandManager, Lumatone::Menu::openSysExMapping, true);

    btnSaveFile.reset(new juce::TextButton("btnSaveFile"));
    addAndMakeVisible(btnSaveFile.get());
    btnSaveFile->setButtonText(juce::translate("SaveFile"));
    btnSaveFile->setCommandToTrigger(commandManager, Lumatone::Menu::saveSysExMappingAs, true);

    btnImportFile.reset(new juce::TextButton("buttonReceive"));
    addAndMakeVisible(btnImportFile.get());
    btnImportFile->setTooltip(juce::translate("ImportTooltip"));
    btnImportFile->setButtonText(juce::translate("Import"));
    btnImportFile->setCommandToTrigger(commandManager, Lumatone::Menu::importSysExMapping, true);

    btnClearSelection.reset(new juce::TextButton("btnClearSelection"));
    addAndMakeVisible(btnClearSelection.get());
    btnClearSelection->setButtonText(juce::translate("x"));
    btnClearSelection->setCommandToTrigger(commandManager, Lumatone::Menu::selectNone, true);

    lblAppName.reset(new Label("lblAppName", getApplicationName()));
    lblAppName->setFont(getAppFonts().getFont(LumatoneEditorFont::FranklinGothic));
    lblAppName->setColour(Label::ColourIds::textColourId, Colour(0xff777777));
    addAndMakeVisible(lblAppName.get());

    lblAppVersion.reset(new Label("lblAppVersion", "v" + getApplicationVersion()));
    lblAppVersion->setFont(getAppFonts().getFont(LumatoneEditorFont::FranklinGothic));
    lblAppVersion->setColour(Label::ColourIds::textColourId, Colour(0xff777777));
    addAndMakeVisible(lblAppVersion.get());

    lblFirmwareVersion = std::make_unique<juce::Label>("lblFirmwareVersion", "");
    lblFirmwareVersion->setFont(getAppFonts().getFont(LumatoneEditorFont::FranklinGothic));
    lblFirmwareVersion->setColour(Label::ColourIds::textColourId, juce::Colour(0xff777777));
    addAndMakeVisible(lblFirmwareVersion.get());

    toggleKeyPropertiesButton.reset(new juce::TextButton("toggleKeyPropertiesButton"));
    addAndMakeVisible(toggleKeyPropertiesButton.get());
    toggleKeyPropertiesButton->setButtonText("i");
    toggleKeyPropertiesButton->setCommandToTrigger(commandManager, Lumatone::Menu::commandIDs::toggleKeyProperties, true);

    addStatusListener(this);
    addEditorListener(this);

    // Initial size
    // setSize(DEFAULTMAINWINDOWWIDTH, DEFAULTMAINWINDOWHEIGHT);

    // Select first board and first key
    // noteEditArea->getOctaveBoardSelectorTab()->setCurrentTabIndex(0, true);
    // The above call is supposed to update changeListener - but apparently doesn't... Call it manually then. XXX
    // changeListenerCallback(noteEditArea->getOctaveBoardSelectorTab());
    // noteEditArea->changeSingleKeySelection(0);

    btnLoadFile->getProperties().set(LumatoneEditorStyleIDs::textButtonIconHashCode, LumatoneEditorIcon::LoadIcon);
    btnSaveFile->getProperties().set(LumatoneEditorStyleIDs::textButtonIconHashCode, LumatoneEditorIcon::SaveIcon);
    btnImportFile->getProperties().set(LumatoneEditorStyleIDs::textButtonIconHashCode, LumatoneEditorIcon::ArrowUp);
    // btnImportFile->getProperties().set(LumatoneEditorStyleIDs::textButtonIconPlacement, LumatoneEditorStyleIDs::TextButtonIconPlacement::RightOfText);

    // Only enable when connected
    btnImportFile->setEnabled(false);

    // DEBUG
    // sectionTabs->setCurrentTabIndex(3);

    // Initialize mapping structure
    //deleteAll();
}

MainContentComponent::~MainContentComponent()
{
    copiedSubBoardData = nullptr;

    lblAppVersion = nullptr;
    lblAppName = nullptr;

    btnImportFile = nullptr;
    btnSaveFile = nullptr;
    btnLoadFile = nullptr;

    // sectionTabs = nullptr;
    sectionTabBar = nullptr;
    lblEditTitle = nullptr;
    lblSelectedKeys = nullptr;

    globalSettingsArea = nullptr;

    mappingSettingsComponent = nullptr;
    keyEditorPanel = nullptr;

    allKeysOverview = nullptr;
    midiEditArea = nullptr;
}

void MainContentComponent::saveStateToPropertiesFile(PropertiesFile*)
{
    // noteEditArea->saveStateToPropertiesFile(propertiesFile);
    // globalSettingsArea->saveStateToPropertiesFile(propertiesFile);
}

// TabbedButtonBar *MainContentComponent::getOctaveBoardSelectorTab()
// {
// 	return  noteEditArea->getOctaveBoardSelectorTab();
// }

// LumatoneAction* MainContentComponent::createDeleteCurrentSectionAction()
// {
// 	auto currentSetSelection = noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex();
// 	if (currentSetSelection >= 0 && currentSetSelection < getOctaveBoardSize())
// 	{
//         auto keySet = LumatoneBoard();
//         return new LumatoneEditAction::SectionEditAction(this, currentSetSelection, keySet);
// 	}
// 	else
// 		return nullptr;
// }

// bool MainContentComponent::copyCurrentSubBoardData()
// {
// 	auto currentSetSelection = noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex();
// 	if (currentSetSelection >= 0 && currentSetSelection < getOctaveBoardSize())
// 	{
// 		*copiedSubBoardData = getBoard(currentSetSelection);
// 		return true;
// 	}
// 	else
// 		return false;
// }

// LumatoneAction* MainContentComponent::createPasteCurrentSectionAction()
// {
// 	auto currentSetSelection = noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex();
// 	if (currentSetSelection >= 0 && currentSetSelection < getNumBoards()
// 		&& !copiedSubBoardData->isEmpty())
// 	{
// 		return new LumatoneEditAction::SectionEditAction(this, currentSetSelection, *copiedSubBoardData);
// 	}
// 	else
// 		return nullptr;
// }

// LumatoneAction* MainContentComponent::createModifiedPasteCurrentSectionAction(CommandID commandID)
// {
//     auto currentSetSelectionIndex = noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex();
//     if (currentSetSelectionIndex >= 0 && currentSetSelectionIndex < getNumBoards()
//         && !copiedSubBoardData->isEmpty())
//     {

// 		auto modifiedSection = getBoard(currentSetSelectionIndex);
// 		auto octaveSize = getOctaveBoardSize();

//         for (int i = 0; i < octaveSize; i++)
//         {
//             LumatoneKey copiedKey = copiedSubBoardData->getKey(i);
//             LumatoneKey currentSectionKey = getKey(currentSetSelectionIndex, i);

//             switch (commandID)
//             {
//             case Lumatone::Menu::commandIDs::pasteOctaveBoardNotes:
//                 currentSectionKey.setNoteOrCC(copiedKey.getMidiNumber());
//                 break;

//             case Lumatone::Menu::commandIDs::pasteOctaveBoardChannels:
//                 currentSectionKey.setChannelNumber(copiedKey.getMidiChannel());
//                 break;

//             case Lumatone::Menu::commandIDs::pasteOctaveBoardColours:
//                 currentSectionKey.setColour(copiedKey.getColour());
//                 break;

//             case Lumatone::Menu::commandIDs::pasteOctaveBoardTypes:
//                 currentSectionKey.setKeyType(copiedKey.getType());
// 				currentSectionKey.setDefaultCCFader(copiedKey.isCCFaderDefault());
//                 break;

//             default:
//                 jassertfalse;
// 				currentSectionKey = copiedKey;
// 				break;
//             }

// 			modifiedSection.setKey(currentSectionKey, i);
//         }

//         return new LumatoneEditAction::SectionEditAction(this, currentSetSelectionIndex, modifiedSection);
//     }
//     else
//         return nullptr;
// }

bool MainContentComponent::canPasteCopiedSubBoard() const
{
    return !copiedSubBoardData->isEmpty();
}

void MainContentComponent::updateDeveloperMode()
{
    // TODO This can just be implemented individually in handleStatePropertyChange
    // curvesArea->setDeveloperMode(inDeveloperMode);
    // globalSettingsArea->setDeveloperMode(inDeveloperMode);
}

void MainContentComponent::connectionStateChanged(ConnectionState stateIn)
{
    btnImportFile->setEnabled(stateIn == ConnectionState::ONLINE);
}

void MainContentComponent::layoutImported(const LumatoneLayout &)
{
    setCurrentFile(juce::File(), false);
    undoManager->clearUndoHistory();
    clearSelectedKeys();
    setHasChangesToSave(false);
}

void MainContentComponent::changeListenerCallback(ChangeBroadcaster *source)
{
    // if (source == noteEditArea->getOctaveBoardSelectorTab())
    // {
    // 	// allKeysOverview->setCurrentSetSelection(noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex());
    // }

    // Probably not the cleanest way to do this
    if (source == sectionTabBar.get())
    {
        // resizeEditSectionTabs();
        setMappingEditorTab(MappingEditorTabs(sectionTabBar->getCurrentTabIndex()));
    }
}

void MainContentComponent::buttonClicked(Button* btn)
{
    ColourViewComponent* colourEdit = dynamic_cast<ColourViewComponent*>(btn);

    if (colourEdit)
    {
        // May be better asynchronous on a timer
        // TerpstraSysExApplication::getApp().loadColourPalettesFromFile();

        auto palettes = getColourPalettes();
        ColourSelectorPanel* paletteWindow = new ColourSelectorPanel(*this);
        paletteWindow->setSize(proportionOfWidth(popupWidth), proportionOfHeight(popupHeight));

        // if (btn == noteEditArea->getColourViewComponent())
        // {
        //     colourEdit = noteEditArea->getColourViewComponent();

        // 	auto colourTextEditor = noteEditArea->getSingleNoteColourTextEditor();
        //     paletteWindow->listenToColourSelection(static_cast<ColourSelectionListener*>(colourTextEditor));

        //     // Shouldn't be necessary when Isomorphic is moved from dev to public
        //     auto isomorphicPanel = noteEditArea->getIsomorphicMassAssignPanel();
        //     if (isomorphicPanel != nullptr)
        //         paletteWindow->listenToColourSelection(static_cast<ColourSelectionListener*>(isomorphicPanel));

        // 	paletteWindow->addColourSelectorToGroup(noteEditArea.get());
        // 	paletteWindow->setCurrentColourSelector(noteEditArea->getSingleNoteColourTextEditor());
        // }

        Rectangle<int> componentArea = colourEdit->getScreenBounds().translated(-getScreenX(), -getScreenY());

        juce::CallOutBox::launchAsynchronously(
            std::unique_ptr<juce::Component>(paletteWindow),
            componentArea,
            this
        );

        // else, a preset button colour button was pressed
        // paletteWindow->listenToColourSelection(colourEdit);
        // TODO: Set swatch # or custom colour as currentSectionKey colour
    }
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll(getEditorLookAndFeel().findColour(LumatoneEditorColourIDs::MediumBackground));

    g.setColour(getEditorLookAndFeel().findColour(LumatoneEditorColourIDs::LightBackground));
    g.fillRect(controlsAreaBackground);
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    int newWidth = getWidth();
    // int newHeight = getHeight();

    contentWidth = proportionOfWidth(contentWidthRatio);
    contentMargin = juce::roundToInt((newWidth - contentWidth) * 0.5);

    // Logo, MIDI edit area and connection state
    int midiAreaHeight = proportionOfHeight(headerHeight);
    midiEditArea->setBounds(0, 0, getWidth(), midiAreaHeight);

    // Bounds for controls, where background is darker
    int footerY = proportionOfHeight(footerAreaY);
    int footerHeight = getHeight() - footerY;

    controlsAreaBackground = getBounds().withTop(proportionOfHeight(controlsAreaY)).withBottom(footerY);
    controlsArea = controlsAreaBackground.withSizeKeepingCentre(contentWidth, controlsAreaBackground.getHeight());

    controlsLabelYPos = controlsArea.getY() - proportionOfHeight(controlsLabelHeight);

    allKeysOverview->setBounds(contentMargin, midiAreaHeight, contentWidth, controlsLabelYPos - midiAreaHeight);

    int btnHeight = juce::roundToInt(getHeight() * fileButtonH);
    int btnMargin = juce::roundToInt(contentWidth * saveLoadMarginW);
    int saveLoadWidth = juce::roundToInt(getWidth() * saveLoadW);
    int btnY = juce::roundToInt(allKeysOverview->getY() + (allKeysOverview->getHeight() * btnYFromImageTop));

    int importWidth = juce::roundToInt(getWidth() * importW);
    int btnBarWidth = saveLoadWidth * 2 + importWidth + btnMargin * 2;

    int halfWidthX = roundToInt(getWidth() * 0.5f);
    int btnBarX = juce::roundToInt(halfWidthX - btnBarWidth * 0.5);

    btnLoadFile->setBounds(btnBarX, btnY, saveLoadWidth, btnHeight);
    btnSaveFile->setBounds(btnLoadFile->getRight() + btnMargin, btnY, saveLoadWidth, btnHeight);
    btnImportFile->setBounds(btnSaveFile->getRight() + btnMargin, btnY, importWidth, btnHeight);;

    int controlHeaderHeight = juce::roundToInt((controlsArea.getY() - controlsLabelYPos) * 0.8f);
    lblEditTitle->setTopLeftPosition(contentMargin, controlsLabelYPos);
    resizeLabelWithHeight(lblEditTitle.get(), controlHeaderHeight);

    int selectedKeyAreaHeight = lblEditTitle->getHeight();
    int clearSize = (selectedKeyAreaHeight * 2) / 3;
    btnClearSelection->setSize(clearSize, clearSize);
    btnClearSelection->setTopRightPosition(controlsArea.getRight(), lblEditTitle->getY() + (selectedKeyAreaHeight - clearSize) / 2);

    resizeLabelWithHeight(lblSelectedKeys.get(), selectedKeyAreaHeight, 0.667f, "___");
    lblSelectedKeys->setTopRightPosition(btnClearSelection->getX() - (btnMargin * 2), lblEditTitle->getY());

    resizeEditSectionTabs();

    keyEditorPanel->setBounds(controlsArea);
    mappingSettingsComponent->setBounds(controlsArea);

    globalSettingsArea->setBounds(getLocalBounds().withTop(roundToInt(getHeight() * footerAreaY)));

    resizeLabelWithHeight(lblAppName.get(), roundToInt(footerHeight * lumatoneVersionHeight), 1.0f, " ");
    lblAppName->setTopLeftPosition(
        proportionOfWidth(lumatoneVersionMarginX),
        footerY + juce::roundToInt((footerHeight - lblAppName->getHeight()) * 0.5f));

    resizeLabelWithHeight(lblAppVersion.get(), roundToInt(lblAppName->getHeight() * 0.75f));
    lblAppVersion->setTopLeftPosition(lblAppName->getRight(), lblAppName->getBottom() - lblAppVersion->getHeight());

    juce::Rectangle<int> lumatoneBounds = allKeysOverview->getLocalBounds().translated(allKeysOverview->getX(), allKeysOverview->getY());
    lblFirmwareVersion->setBounds(lumatoneBounds.getX(), lumatoneBounds.getBottom(), contentWidth, juce::roundToInt((float)btnHeight * 0.7f));
}

void MainContentComponent::resizeEditSectionTabs()
{
    int sectionTabsMargin = proportionOfWidth(sectionTabsMarginW);
    int sectionTabsWidth = 0;

    juce::StringArray sectionTabNames = sectionTabBar->getTabNames();

    auto font = getEditorLookAndFeel().getTabBarFont((float)sectionTabBar->getHeight() * 0.7f);
    for (int i = 0; i < sectionTabNames.size(); i++)
    {
        sectionTabsWidth += font.getStringWidth(sectionTabNames[i]) + sectionTabsMargin;
    }

    int leftMargin = roundToInt(sectionTabsMargin * 0.5f);
    int tabHeight = controlsArea.getY() - controlsLabelYPos;
    sectionTabBar->setBounds(lblEditTitle->getRight() + leftMargin, controlsLabelYPos, sectionTabsWidth, tabHeight);
}

void MainContentComponent::setMappingEditorTab(MappingEditorTabs tabIndex)
{
    if (editorTabComponent != nullptr)
        editorTabComponent->setVisible(false);

    switch (tabIndex)
    {
    case MappingEditorTabs::AutoGenerator:
        // TODO
    case MappingEditorTabs::Advanced:
        // TODO
    case MappingEditorTabs::MappingSettings:
        editorTabComponent = mappingSettingsComponent.get();
        break;
    case MappingEditorTabs::KeyEditor:
    default:
        editorTabComponent = keyEditorPanel.get();
        break;
    }

    editorTabComponent->setVisible(true);
}

void MainContentComponent::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property)
{
    LumatoneEditorState::handleStatePropertyChange(stateIn, property);

    if (property == LumatoneEditorProperty::DeveloperModeOn)
    {
        updateDeveloperMode();
    }
    else if (property == LumatoneApplicationProperty::NumKeySelected)
    {
        juce::String append = stateIn[property].toString() == "1" ? " Key" : " Keys";
        append += " Selected";
        lblSelectedKeys->setText(stateIn[property].toString() + append, juce::NotificationType::dontSendNotification);
    }
    else if (property == LumatoneStateProperty::LastConnectedFirmwareVersion)
    {
        auto string = juce::String("Firmware version: " + getFirmwareVersion().toDisplayString());
        lblFirmwareVersion->setText(string, juce::NotificationType::sendNotification);

        requestDeviceGlobalSettings();
    }
    else if (property == LumatoneEditorProperty::ShowKeyProperties)
    {
        allKeysOverview->setShowKeyProperties((bool)stateIn[property]);
    }
}
