    /*
  ==============================================================================

    key_component.cpp
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "key_component.h"

//==============================================================================

LumatoneKeyDisplay::LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex, const LumatoneKey& keyDataIn)
    : LumatoneKeyContext(keyDataIn, newBoardIndex, newKeyIndex)
    , Component("LumatoneKeyDisplay_" + LumatoneKeyCoord::toString(newBoardIndex, newKeyIndex))
{
    boardIndex = newBoardIndex;
    keyIndex = newKeyIndex;

    renderMode = LumatoneComponentRenderMode::GraphicInteractive;
    clearUiState();
}

LumatoneKeyDisplay::~LumatoneKeyDisplay()
{
}

void LumatoneKeyDisplay::setRenderMode(LumatoneComponentRenderMode modeIn)
{
    renderMode = modeIn;
}

void LumatoneKeyDisplay::paint(juce::Graphics& g)
{
    juce::Colour hexagonColour = getColour();
    // juce::Colour hexagonColour = findColour(LumatoneKeyEdit::backgroundColourId).overlaidWith(getKeyColour());
    // if (hexagonColour.getPerceivedBrightness() >= 0.6f)
    //    hexagonColour = hexagonColour.darker((1.0f - hexagonColour.getPerceivedBrightness()));

    // set color
    juce::Colour drawColour = hexagonColour;

    switch (renderMode)
    {
    default:
        break;

    case LumatoneComponentRenderMode::ShapeInteractive:
    case LumatoneComponentRenderMode::GraphicInteractive:
    case LumatoneComponentRenderMode::MaxRes:
        if(isSelected)
        {
            // drawColour = drawColour.overlaidWith(juce::Colours::slateblue.withAlpha(0.2f));
        }
        if (isNoteOn || isClicked)
        {
            drawColour = hexagonColour.contrasting(0.5f);
        }
        else if (mouseIsOver && !isClicked)
        {
            drawColour = drawColour.contrasting(0.25f);
        }
        break;
    }

    // draw
    switch (renderMode)
    {
    case LumatoneComponentRenderMode::Shape:
    case LumatoneComponentRenderMode::ShapeInteractive:
        break; // TODO

    default:
    case LumatoneComponentRenderMode::Graphic:
    case LumatoneComponentRenderMode::GraphicInteractive:
    {
        g.setColour(drawColour);

        if (colourGraphic.isValid())
        {
            g.drawImageAt(colourGraphic, 0, 0, true);
        }

        if (shadowGraphic.isValid())
        {
            g.drawImageAt(shadowGraphic, 0, 0);
        }
        break;
    }

    case LumatoneComponentRenderMode::MaxRes:
    {
        if (drawColour != hexagonColour && shadowGraphic.isValid())
        {
            g.setColour(drawColour.overlaidWith(juce::Colours::black.withAlpha(0.88f)));
            g.drawImageAt(shadowGraphic, 0, 0, true);
        }
        break;
    }
    }

    if (isSelected)
    {
        g.setColour(juce::Colour(0xff333333));
        g.fillEllipse(selectBounds);
    }

    if (showMidiInfo)
    {
        g.setColour(juce::Colour(0xff333333));
        g.drawFittedText(juce::String(getMidiChannel()), channelNumberArea, juce::Justification::centred, 1);
        g.drawFittedText(juce::String(getMidiNumber()), midiNumberArea, juce::Justification::centred, 1);
    }

}

void LumatoneKeyDisplay::resized()
{
    float dotSize = jmin(getWidth(), getHeight()) * selectedDotScalar;
    float dotOffset = dotSize * 0.5f;
    selectBounds = juce::Rectangle<float>(getWidth() * 0.5f - dotOffset
                                        , getHeight() * 0.5f - dotOffset
                                        , dotSize
                                        , dotSize);

    midiNumberHeight = (int)((0.5f-channelNumberYHeight)*getHeight());
    noteNumberY = juce::roundToInt(getHeight() * 0.5);
    channelNumberY = noteNumberY - midiNumberHeight;

    channelNumberArea = juce::Rectangle<int>(0, channelNumberY, getWidth(), midiNumberHeight);
    midiNumberArea = juce::Rectangle<int>(0, noteNumberY, getWidth(), midiNumberHeight);

    if (showMidiInfo && isSelected)
    {
        int infoMarginY = (int)((float)getHeight()*selectedDotScalar);
        channelNumberArea = channelNumberArea.withTrimmedBottom(infoMarginY);
        midiNumberArea = midiNumberArea.withTrimmedTop(infoMarginY);
    }
}

void LumatoneKeyDisplay::parentHierarchyChanged()
{
    //if (getParentComponent())
    //{
    //    removeMouseListener(this);
    //}
    //else
    //{
    //    addMouseListener(this, true);
    //}
}

void LumatoneKeyDisplay::mouseEnter(const juce::MouseEvent& e)
{
    mouseIsOver = true;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::mouseExit(const juce::MouseEvent& e)
{
    mouseIsOver = false;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::mouseDown(const juce::MouseEvent& e)
{
    //DBG("KEY MOUSE DOWN");

    mouseIsOver = true;

    if (e.mods.isRightButtonDown())
    {
        // Right mouse click: popup menu
        //juce::PopupMenu menu;
        //TerpstraSysExApplication::getApp().getMainMenu()->createEditMenu(menu);
        //menu.showMenuAsync(PopupMenu::Options());
    }

    // TODO integrate interaction through LumatoneController

    //else
    //{
    //    // NoteOn MIDI message
    //    auto keyData = getKeyData();
    //    if (keyData != nullptr && keyData->channelNumber > 0)
    //    {
    //        if (keyData->keyType == LumatoneKeyType::velocityInterval)
    //        {
    //            // Send "note on" event
    //            //TerpstraSysExApplication::getApp().getMidiDriver().sendNoteOnMessage(keyData->noteNumber, keyData->channelNumber, 60);
    //        }
    //        // ToDo if keyType is "continuous controller": send controller event?
    //    }
    //}
}

void LumatoneKeyDisplay::mouseUp(const juce::MouseEvent& e)
{
    //DBG("KEY MOUSE UP");
    if (!isMouseOver() && mouseIsOver)
    {
        mouseIsOver = false;
    }

    repaintIfInteractive();
}

//void LumatoneKeyDisplay::handleMidiMessage(const juce::MidiMessage& msg)
//{
//    auto keyData = getKeyData();
//    if (keyData != nullptr && msg.getChannel() == keyData->channelNumber && msg.getNoteNumber() == keyData->noteNumber)
//    {
//        isMouseOver = msg.isNoteOn();
//        repaint();
//    }
//}

void LumatoneKeyDisplay::mouseMove(const juce::MouseEvent& e)
{
    if (!mouseIsOver)
    {
        mouseIsOver = true;
        repaintIfInteractive();
    }
}

void LumatoneKeyDisplay::mouseDrag(const juce::MouseEvent& e)
{
    //DBG("KEY DRAG on " + getCoord().toString() + ": " + e.getPosition().toString
    //mouseDragInternal(e);
}

void LumatoneKeyDisplay::startDrag()
{
    if (!isClicked)
    {
        isClicked = true;
        noteOn();
    }
}

void LumatoneKeyDisplay::endDrag()
{
    if (isClicked)
    {
        isClicked = false;

        if (noteOffMode != NoteOffModifier::Sustain)
        {
            noteOff();
        }

    }
}

void LumatoneKeyDisplay::setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn)
{
    colourGraphic = colourGraphicIn;
    shadowGraphic = shadowGraphicIn;
}

void LumatoneKeyDisplay::setLumatoneKey(const LumatoneKey &lumatoneKey, juce::Colour displayColour)
{
    LumatoneKey::operator=(lumatoneKey);
    LumatoneKey::setColour(displayColour);
    repaint();
}

void LumatoneKeyDisplay::setSelected(bool selected)
{
    isSelected = selected;
    if (showMidiInfo)
        resized();
    // else
    repaintIfInteractive();
}

void LumatoneKeyDisplay::setShowMidiInfo(bool showInfo)
{
    showMidiInfo = showInfo;
    repaint();
}

void LumatoneKeyDisplay::clearUiState()
{
    isSelected = false;
    mouseIsOver = false; // mouseMove doesn't fix this atm
    isNoteOn = false;
    isClicked = false;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::repaintIfInteractive()
{
    switch (renderMode)
    {
    default:
        break;
    case LumatoneComponentRenderMode::ShapeInteractive:
    case LumatoneComponentRenderMode::GraphicInteractive:
    case LumatoneComponentRenderMode::MaxRes:
        if (mouseIsOver != isMouseOver())
        {
            mouseIsOver = isMouseOver();
        }

        repaint();
        break;
    }
}

void LumatoneKeyDisplay::noteOn()
{
    isNoteOn = true;
    isClicked = true;

    repaintIfInteractive();
}

void LumatoneKeyDisplay::noteOff()
{
    isNoteOn = false;
    isClicked = false;

    repaintIfInteractive();
}
