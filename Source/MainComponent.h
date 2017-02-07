/*
  ==============================================================================

    This file was auto-generated!
	Created: XXX.2014
	Author:  hsstraub

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "ViewComponents.h"
#include "KeyboardDataStructure.h"
#include "TerpstraMidiDriver.h"
#include "MidiEditArea.h"
#include "NoteAssignTab.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent : public Component, public ButtonListener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

	// Transfer of data
	void setData(TerpstraKeyMapping& newData);
	void getData(TerpstraKeyMapping& newData);
	TerpstraKeyMapping&	getMappingInEdit() { return this->mappingData; }


	// GUI implementation
    void paint (Graphics&);
    void resized();
	void buttonClicked(Button *button);
	void mouseDown(const MouseEvent &event);

private:
	void changeSetSelection(int newSelection, bool forceRefresh = false);
	void changeSingleKeySelection(int newSelection);

private:
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)

	//==============================================================================
	// GUI components

	// Buttons for SysEx commands
	// XXX

	// Sets of 56 keys
	TerpstraKeySetEdit* terpstraSetSelectors[NUMBEROFBOARDS];

	// Editing single keys (of the selected 56-key set)
	TerpstraKeyEdit*	terpstraKeyFields[TERPSTRABOARDSIZE];

	// Midi devices
	MidiEditArea*		midiEditArea;

	// Edit fields for setting key and button parameters
	TabbedComponent*	editArea;
	NoteAssignTab*		noteAssignTab;

	//==============================================================================
	// Data
	TerpstraKeyMapping	mappingData;
	int					currentSetSelection;
	int					currentSingleKeySelection;

	// MIDI connection
	TerpstraMidiDriver	midiDriver;
};


#endif  // MAINCOMPONENT_H_INCLUDED
