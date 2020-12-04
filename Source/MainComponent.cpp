/*
  ==============================================================================

  MainComponent.cpp
  Created: XXX.2014
  Author:  hsstraub

  ==============================================================================
*/

#include "MainComponent.h"
#include "ViewConstants.h"
#include "Main.h"


//==============================================================================
MainContentComponent::MainContentComponent()
	: copiedSubBoardData()
{
	// Midi input + output
	midiEditArea.reset(new MidiEditArea());
	addAndMakeVisible(midiEditArea.get());

	// All keys overview
	allKeysOverview.reset(new AllKeysOverview());
	addAndMakeVisible(allKeysOverview.get());

	// Edit function area
	noteEditArea.reset(new NoteEditArea());
	addAndMakeVisible(noteEditArea.get());
	noteEditArea->getOctaveBoardSelectorTab()->addChangeListener(this);

	generalOptionsArea.reset(new GeneralOptionsDlg());
	addAndMakeVisible(generalOptionsArea.get());

	curvesArea.reset(new CurvesArea());
	addAndMakeVisible(curvesArea.get());

	globalSettingsArea.reset(new GlobalSettingsArea());
	addAndMakeVisible(globalSettingsArea.get());

	TerpstraSysExApplication::getApp().getMidiDriver().addListener(this);

	// Initial size
	setSize(DEFAULTMAINWINDOWWIDTH, DEFAULTMAINWINDOWHEIGHT);

	// Select first board and first key
	noteEditArea->getOctaveBoardSelectorTab()->setCurrentTabIndex(0, true);
	// The above call is supposed to update changeListener - but apaprently doesn't... Call it manually then. XXX
	changeListenerCallback(noteEditArea->getOctaveBoardSelectorTab());

	noteEditArea->changeSingleKeySelection(0);
}

MainContentComponent::~MainContentComponent()
{
    TerpstraSysExApplication::getApp().getMidiDriver().removeListener(this);

	midiEditArea = nullptr;
	allKeysOverview = nullptr;
	noteEditArea = nullptr;

	generalOptionsArea = nullptr;
	curvesArea = nullptr;
	globalSettingsArea = nullptr;
}

void MainContentComponent::restoreStateFromPropertiesFile(PropertiesFile* propertiesFile)
{
	setSize(
		propertiesFile->getIntValue("MainWindowWidth", DEFAULTMAINWINDOWWIDTH),
		propertiesFile->getIntValue("MainWindowHeight", DEFAULTMAINWINDOWHEIGHT));

	noteEditArea->restoreStateFromPropertiesFile(propertiesFile);
}

void MainContentComponent::saveStateToPropertiesFile(PropertiesFile* propertiesFile)
{
	propertiesFile->setValue("MainWindowWidth", getWidth());
	propertiesFile->setValue("MainWindowHeight", getHeight());

	noteEditArea->saveStateToPropertiesFile(propertiesFile);
}

// Set the current mapping to be edited to the value passed in parameter
void MainContentComponent::setData(TerpstraKeyMapping& newData, bool withRefresh)
{
	mappingData = newData;

	noteEditArea->onSetData(newData);

	if (withRefresh)
	{
		refreshAllKeysOverview();
		noteEditArea->refreshKeyFields();
		generalOptionsArea->loadFromMapping();
		// ToDo curves
	}
}

void MainContentComponent::deleteAll(bool withRefresh)
{
	TerpstraKeyMapping keyMapping;
	setData(keyMapping, withRefresh);
}

// Copy the edited mapping to the variable passed as parameter
void MainContentComponent::getData(TerpstraKeyMapping& newData)
{
	newData = mappingData;
}

bool MainContentComponent::deleteCurrentSubBoardData()
{
	//if (currentSetSelection >= 0 && currentSetSelection < NUMBEROFBOARDS)
	//{
	//	// Delete subboard data
	//	mappingData.sets[currentSetSelection] = TerpstraKeys();

	//	// Refresh display
	//	changeSetSelection(currentSetSelection, true);

	//	// Mark that there are changes
	//	TerpstraSysExApplication::getApp().setHasChangesToSave(true);

	//	return true;
	//}
	//else
	//todo
		return false;
}

bool MainContentComponent::copyCurrentSubBoardData()
{
	//if (currentSetSelection >= 0 && currentSetSelection < NUMBEROFBOARDS)
	//{
	//	copiedSubBoardData = mappingData.sets[currentSetSelection];
	//	return true;
	//}
	//else
	//ToDo
		return false;
}

bool MainContentComponent::pasteCurrentSubBoardData()
{
	//if (currentSetSelection >= 0 && currentSetSelection < NUMBEROFBOARDS)
	//{
	//	if (!copiedSubBoardData.isEmpty())
	//	{
	//		mappingData.sets[currentSetSelection] = copiedSubBoardData;

	//		// Refresh display
	//		changeSetSelection(currentSetSelection, true);

	//		// Mark that there are changes
	//		TerpstraSysExApplication::getApp().setHasChangesToSave(true);
	//	}
	//	return true;
	//}
	//else
	// ToDO
		return false;
}

void MainContentComponent::midiMessageReceived(const MidiMessage& message)
{
    if (TerpstraSysExApplication::getApp().getMidiDriver().messageIsTerpstraConfigurationDataReceptionMessage(message))
    {
        auto sysExData = message.getSysExData();

        int boardNo = sysExData[3];
        jassert(boardNo >= 1 && boardNo <= NUMBEROFBOARDS);
        auto midiCmd = sysExData[4];
        auto answerState = sysExData[5];

        if (answerState == TerpstraMidiDriver::ACK)
        {
            // After the answer state byte there must be 55 bytes of data (one for each key)
            jassert(message.getSysExDataSize() >= TERPSTRABOARDSIZE + 6); // ToDo display error otherwise

            for (int keyIndex = 0; keyIndex < TERPSTRABOARDSIZE; keyIndex++)
            {
                auto newValue = sysExData[6 + keyIndex];

                TerpstraKey& keyData = this->mappingData.sets[boardNo-1].theKeys[keyIndex];

                switch(midiCmd)
                {
                case GET_RED_LED_CONFIG:
                {
                    auto theColour = Colour(keyData.colour);
                    theColour = Colour(newValue, theColour.getGreen(), theColour.getBlue());
                    keyData.colour = theColour.toDisplayString(false).getHexValue32();
                    break;
                }

                case GET_GREEN_LED_CONFIG:
                {
                    auto theColour = Colour(keyData.colour);
                    theColour = Colour(theColour.getRed(), newValue, theColour.getBlue());
                    keyData.colour = theColour.toDisplayString(false).getHexValue32();
                    break;
                }

                case GET_BLUE_LED_CONFIG:
                {
                    auto theColour = Colour(keyData.colour);
                    theColour = Colour(theColour.getRed(), theColour.getGreen(), newValue);
                    keyData.colour = theColour.toDisplayString(false).getHexValue32();
                    break;
                }

                case GET_CHANNEL_CONFIG:
                    keyData.channelNumber = newValue;
                    break;

                case GET_NOTE_CONFIG:
                    keyData.noteNumber = newValue;
                    break;

                case GET_KEYTYPE_CONFIG:
                    keyData.keyType = (TerpstraKey::KEYTYPE)newValue;
                    break;

                default:
                    jassertfalse;   // Should not happen
                    break;
                }
            }

			refreshAllKeysOverview();
        }
    }
}

void MainContentComponent::changeListenerCallback(ChangeBroadcaster *source)
{
	if (source == noteEditArea->getOctaveBoardSelectorTab())
	{
		allKeysOverview->setCurrentSetSelection(noteEditArea->getOctaveBoardSelectorTab()->getCurrentTabIndex());
	}
}

void MainContentComponent::paint (Graphics& g)
{
	g.fillAll(findColour(ResizableWindow::backgroundColourId));
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
	int newWidth = getWidth();
	int newHeight = getHeight();

	// Logo, MIDI edit area and connection state
	int midiAreaWidth = midiEditArea->getWidth();
	int midiAreaHeight = midiEditArea->getHeight();
	int midiAreaXPos = jmax(newWidth - midiAreaWidth, 0);
	midiEditArea->setBounds(midiAreaXPos, 0, midiAreaWidth, midiAreaHeight);

	// All keys overview/virtual keyboard playing
	// New height of subset field area, with minimal value
	int noteEditAreaWidth = noteEditArea->getWidth();
	int noteEditAreaHeight = noteEditArea->getHeight();

	int newKeysOverviewAreaHeight = jmax(newHeight - midiAreaHeight - noteEditAreaHeight, MINIMALTERPSTRAKEYSETAREAHEIGHT);

	allKeysOverview->setBounds(0, midiAreaHeight, newWidth, newKeysOverviewAreaHeight);

	// Edit function/single key field area
	noteEditArea->setBounds(0, midiAreaHeight + newKeysOverviewAreaHeight, noteEditAreaWidth, noteEditAreaHeight);

	int optionsAreaWidth = jmax(newWidth - noteEditAreaWidth, MINIMALCURVESAREAWIDTH);
	
	int generalOptionsYPos = allKeysOverview->getBottom() + OCTAVEBOARDTABHEIGHT;
	generalOptionsArea->setBounds(noteEditAreaWidth, generalOptionsYPos, generalOptionsArea->getWidth(), generalOptionsArea->getHeight());

	int curvesAreaYPos = generalOptionsArea->getBottom();
	int curvesAreaHeight = jmax(newHeight - curvesAreaYPos - globalSettingsArea->getHeight(), MINIMALCURVESAREAHEIGHT);
	curvesArea->setBounds(noteEditAreaWidth, curvesAreaYPos, optionsAreaWidth, curvesAreaHeight);

	globalSettingsArea->setBounds(noteEditAreaWidth, curvesArea->getBottom(), globalSettingsArea->getWidth(), globalSettingsArea->getHeight());
}

void MainContentComponent::refreshAllKeysOverview()
{
	allKeysOverview->repaint();
}