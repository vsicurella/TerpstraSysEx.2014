/*
  ==============================================================================

    key_component.h
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_KEY_COMPONENT_H
#define LUMATONE_KEY_COMPONENT_H

#include <JuceHeader.h>
#include "../data/lumatone_context.h"
#include "../graphics/resampler/image_resampler.h"
#include "../mapping/lumatone_tiling.h"

enum class LumatoneComponentRenderMode
{
    NoDisplay = -1,

    Shape = 0x000010,
    ShapeInteractive = 0x000011,

    Graphic = 0x000020,
    GraphicInteractive = 0x000021,

    MaxRes = 0x000030,
};

// Representation of a key inside the overview
class LumatoneKeyDisplay : public LumatoneKeyContext,
                           public juce::Component
{
public:
    enum class NoteOffModifier
    {
        None = 0,
        Sustain,
        Legato
    };

public:
    LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex, const LumatoneKey& keyData=LumatoneKey());
    ~LumatoneKeyDisplay() override;

    LumatoneComponentRenderMode getRenderMode() const { return renderMode; }
    void setRenderMode(LumatoneComponentRenderMode uiModeIn);

    void paint(juce::Graphics&) override;
    void resized() override;

    void setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn);

    void setLumatoneKey(const LumatoneKey& lumatoneKey, juce::Colour displayColour);

    int getBoardIndex() const { return boardIndex; }
    int getKeyIndex() const { return keyIndex; }
    LumatoneKeyCoord getCoord() const { return LumatoneKeyCoord(boardIndex, keyIndex); }

    void setSelected(bool selected);

    void setShowMidiInfo(bool showInfo);

    bool selected() const { return isSelected; }
    bool clicked() const { return isClicked; }
    bool mouseOver() const { return mouseIsOver; }

    void startDrag();
    void endDrag();
    void clearUiState();

    void noteOn();
    void noteOff();

private:

    void repaintIfInteractive();

public:

    void parentHierarchyChanged() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumatoneKeyDisplay)

    LumatoneComponentRenderMode renderMode;
    LumatoneKeyDisplay::NoteOffModifier noteOffMode;

    bool mouseIsOver = false;
    bool isSelected = false;
    bool isClicked = false;
    bool isNoteOn = false;

    juce::Image colourGraphic;
    juce::Image shadowGraphic;

    juce::Rectangle<float> selectBounds;

    bool showMidiInfo = false;

    const float selectedDotScalar = 0.15f;

    int midiNumberHeight;
    int channelNumberY;
    int noteNumberY;
    const float channelNumberYHeight = 0.1f;
    const float channelNumberFontScalar = 0.6;

    juce::Rectangle<int> channelNumberArea;
    juce::Rectangle<int> midiNumberArea;
};

#endif LUMATONE_KEY_COMPONENT_H
