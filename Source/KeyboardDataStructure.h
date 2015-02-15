/*
==============================================================================

KeyboardDataStructure.h
Created: XXX
Author:  hsstraub

==============================================================================
*/

#ifndef KEYBOARDDATASTRUCTURE_INCLUDED
#define KEYBOARDDATASTRUCTURE_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


// Mapping of one key
class TerpstraKey {
public:
	TerpstraKey() { noteNumber = 0; channelNumber = 0; colour = 0; };

	// 0 is a valid note number, but channel 0 doesn't exist 
	bool isEmpty() const { return channelNumber == 0; }

public:
	int noteNumber;
	int channelNumber;
	int colour;
};

// Subset of 56 keys
#define TERPSTRABOARDSIZE 56

struct TerpstraKeys {
	TerpstraKey		theKeys[TERPSTRABOARDSIZE];
	int				board_idx;
	int				key_idx;

	TerpstraKeys();
};

/*
==============================================================================
A complete key mapping
==============================================================================
*/
#define NUMBEROFBOARDS 5
class TerpstraKeyMapping
{
public:
	TerpstraKeyMapping();
	
	void clearAll();

	void fromStringArray(const StringArray& stringArray);
	StringArray toStringArray();

public:
	TerpstraKeys	sets[NUMBEROFBOARDS];
};


#endif