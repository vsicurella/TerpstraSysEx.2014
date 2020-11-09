/*
  ==============================================================================

    OctaveBoardComponent.cpp
    Created: 11 Jan 2020 6:54:03pm
    Author:  hsstraub

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "OctaveBoardComponent.h"
#include "Main.h"
#include "BoardGeometry.h"

// Geometry settings
static TerpstraBoardGeometry	boardGeometry;


//==============================================================================
KeyMiniDisplayInsideOctaveBoardComponent::KeyMiniDisplayInsideOctaveBoardComponent(int newKeyIndex)
{
	// In your constructor, you should add any child components, and
	// initialise any special settings that your component needs.
	keyIndex = newKeyIndex;

}

KeyMiniDisplayInsideOctaveBoardComponent::~KeyMiniDisplayInsideOctaveBoardComponent()
{
}

void KeyMiniDisplayInsideOctaveBoardComponent::paint(Graphics& g)
{
	// ToDO IsSelected new functionality?
	//bool isSelected = ((OctaveBoardComponent*)getParentComponent())->getIsSelected();

    Colour hexagonColour = findColour(TerpstraKeyEdit::backgroundColourId).overlaidWith(getKeyColour()
     //   .withAlpha(isSelected ? TERPSTRASINGLEKEYCOLOURALPHA : TERPSTRASINGLEKEYCOLOURUNSELECTEDMINIALPHA));
		.withAlpha(TERPSTRASINGLEKEYCOLOURALPHA));
	g.setColour(hexagonColour);
	g.fillPath(hexPath);

	Colour lineColour = findColour(TerpstraKeyEdit::outlineColourId);
	//if ( !isSelected)
 //       lineColour = lineColour.withAlpha(TERPSTRASINGLEKEYCOLOURUNSELECTEDMINIALPHA);
	g.setColour(lineColour);
	g.strokePath(hexPath, PathStrokeType(1));
}

void KeyMiniDisplayInsideOctaveBoardComponent::resized()
{
	float w = this->getWidth();
	float h = this->getHeight();

	float marginOffset = 1.5;

	// recalculate position and size of hexagon
	hexPath.clear();
	hexPath.startNewSubPath(w / 2.0f, 0);
	hexPath.lineTo(w, h / 4.0f);
	hexPath.lineTo(w, 3.0f * h / 4.0f);
	hexPath.lineTo(w / 2.0f, h);
	hexPath.lineTo(0, 3.0f * h / 4.0f);
	hexPath.lineTo(0, h / 4.0f);
	hexPath.closeSubPath();

	// Rotate slightly counterclockwise around the center
	AffineTransform transform = AffineTransform::translation(-w / 2.0f, -h / 2.0f);
	transform = transform.rotated(TERPSTRASINGLEKEYROTATIONANGLE);
	transform = transform.translated(w / 2.0f, h / 2.0f);

	hexPath.applyTransform(transform);
	hexPath.scaleToFit(marginOffset, marginOffset, w - 2*marginOffset, h - 2*marginOffset, true);
}

void KeyMiniDisplayInsideOctaveBoardComponent::mouseDown(const MouseEvent& e)
{
	Component::mouseDown(e);

	// If right mouse click: popup menu (implementation in parent component)
	if (e.mods.isRightButtonDown())
		getParentComponent()->mouseDown(e);
}


Colour KeyMiniDisplayInsideOctaveBoardComponent::getKeyColour()
{
    if ( keyIndex >= 0 && keyIndex < TERPSTRABOARDSIZE)
    {
        TerpstraKeys* pCurrentOctaveBoardData = ((OctaveBoardComponent*)getParentComponent())->getKeyData();
        if ( pCurrentOctaveBoardData != nullptr)
            return Colour(pCurrentOctaveBoardData->theKeys[keyIndex].colour);
    }

    return findColour(TerpstraKeyEdit::backgroundColourId);
}

//==============================================================================
OctaveBoardComponent::OctaveBoardComponent(int newOctaveBoardIndex)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    octaveBoardIndex = newOctaveBoardIndex;

	for (int i = 0; i < TERPSTRABOARDSIZE; i++)
	{
		keyMiniDisplay[i].reset(new KeyMiniDisplayInsideOctaveBoardComponent(i));
		addAndMakeVisible(keyMiniDisplay[i].get());
	}
}

OctaveBoardComponent::~OctaveBoardComponent()
{
	for (int i = 0; i < TERPSTRABOARDSIZE; i++)
	{
		keyMiniDisplay[i] = nullptr;
	}
}

void OctaveBoardComponent::paint (Graphics& g)
{
	if ( isSelected)
	{ 
		Path bottomLine;
		bottomLine.startNewSubPath(0.0f, float(getBottom()-3));
		bottomLine.lineTo(float(getWidth()), float(getBottom()-3));

		Colour lineColour = findColour(TerpstraKeyEdit::outlineColourId);
		g.setColour(lineColour);
		g.strokePath(bottomLine, PathStrokeType(3.0f));
	}
}

void OctaveBoardComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

	int newHeight = getHeight();
	int newWidth = getWidth();

	// Single key field size
	// ToDo better logic
	int newSingleKeySize = jmin(newWidth*2/17, newHeight/8);

	// Transformation Rotate slightly counterclockwise
	float x = 0.0f;
	float y = 0.0f;
	AffineTransform transform = AffineTransform::rotation(TERPSTRASINGLEKEYROTATIONANGLE);

	int keyIndex = 0;
	int mostBottomKeyPos = 0;

	// Rows
	int rowCount = boardGeometry.horizontalLineCount();
	for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
	{
		float xbasepos;
		if (rowIndex % 2 == 0)
			xbasepos = 0.0f;
		else
			xbasepos = 0.0f + newSingleKeySize / 2.0f;

		float ybasepos = 0.0f + 3.0f * rowIndex * newSingleKeySize / 4.0f;

		int subBoardRowSize = boardGeometry.horizontalLineSize(rowIndex);
		for (int posInRow = 0; posInRow < subBoardRowSize; posInRow++)
		{
			x = xbasepos + (boardGeometry.firstColumnOffset(rowIndex) + posInRow)*newSingleKeySize;
			y = ybasepos;
			transform.transformPoint(x, y);
			keyMiniDisplay[keyIndex]->setBounds(roundToInt(x), roundToInt(y), newSingleKeySize, newSingleKeySize);

			mostBottomKeyPos = jmax(mostBottomKeyPos, keyMiniDisplay[keyIndex]->getBottom());

			keyIndex++;
		}
	}

	// Move key fields to bottom
	if ( mostBottomKeyPos < newHeight)
	{
		int ydispacement = newHeight - mostBottomKeyPos;
		int maxKeyIndex = keyIndex;
		for (keyIndex = 0; keyIndex < maxKeyIndex; keyIndex++)
			keyMiniDisplay[keyIndex]->setTopLeftPosition(
				juce::Point<int>(keyMiniDisplay[keyIndex]->getX(), keyMiniDisplay[keyIndex]->getY() + ydispacement));
	}
}

void OctaveBoardComponent::mouseDown(const MouseEvent& e)
{
	Component::mouseDown(e);

	// If right mouse click: popup menu
	if (e.mods.isRightButtonDown())
	{
		PopupMenu menu;
		TerpstraSysExApplication::getApp().getMainMenu()->createEditMenu(menu);
		menu.show();
	}
}

void OctaveBoardComponent::setIsSelected(bool newValue)
{
	// ToDO new funcitonality
	if (newValue != isSelected)
	{
		isSelected = newValue;
		repaint();
	}
}

TerpstraKeys* OctaveBoardComponent::getKeyData()
{
     if ( octaveBoardIndex >= 0 && octaveBoardIndex < TERPSTRABOARDSIZE)
    {
        return &((MainContentComponent*)(getParentComponent()))->getMappingInEdit().sets[octaveBoardIndex];
    }
    else
        return nullptr;
}
