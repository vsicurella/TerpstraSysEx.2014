#include "LumatoneEditorKeyboard.h"
// #include "../data/LumatoneE/ditorState.h"

#include "../actions/KeySelectionControlActions.h"


LumatoneEditorKeyboardComponent::LumatoneEditorKeyboardComponent(const LumatoneEditorState &stateIn)
    : LumatoneEditorState("LumatoneEditorKeyboardComponent", stateIn)
    , LumatoneKeyboardComponent((const LumatoneApplicationState&)(stateIn))
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    // , editorState(stateIn)
{
    addColourSelectionBroadcaster(this);
}

LumatoneEditorKeyboardComponent::~LumatoneEditorKeyboardComponent()
{
    removeColourSelectionBroadcaster(this);
}

void LumatoneEditorKeyboardComponent::mouseMove(const juce::MouseEvent &e)
{
    LumatoneKeyboardComponent::mouseMove(e);
}

void LumatoneEditorKeyboardComponent::mouseDown(const juce::MouseEvent &e)
{
    auto key = getKeyFromMouseEvent(e);
    if (key)
    {
        LumatoneKeyboardComponent::mouseDownInternal(e, key);

        LumatoneKeyCoord keyCoord = key->getKeyCoord();
        LumatoneEditorState::Controller::DoEditKeyDownAction(keyCoord, e.mods);
    }
}

void LumatoneEditorKeyboardComponent::mouseUp(const juce::MouseEvent &e)
{
    LumatoneKeyboardComponent::mouseUp(e);
    mouseWasDragging = false;
}

void LumatoneEditorKeyboardComponent::mouseDrag(const juce::MouseEvent &e)
{
    LumatoneKeyboardComponent::mouseDrag(e);
    mouseWasDragging = true;
}

void LumatoneEditorKeyboardComponent::mouseDragInternalOnNewKey(const juce::MouseEvent &e, LumatoneKeyDisplay *key)
{
    LumatoneKeyCoord keyCoord = key->getKeyCoord();
    // bool isSelected = isKeySelected(keyCoord);

    bool undoable = true;
    bool newTransaction = !mouseWasDragging;

    bool actionDone = DoEditKeyDownAction(keyCoord, e.mods);
    if (actionDone && !e.mods.isCommandDown())
    {
        if (LumatoneEditorState::getSelectedKeys()->size() > 0)
        {
            selectedColour = getEditSelectionData().colour;
            selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, selectedColour);
        }
    }
}

bool LumatoneEditorKeyboardComponent::keyStateChanged(bool isKeyDown)
{
    return LumatoneKeyboardComponent::keyStateChanged(isKeyDown);
}

bool LumatoneEditorKeyboardComponent::keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent)
{
    return LumatoneKeyboardComponent::keyPressed(key, originatingComponent);
}

void LumatoneEditorKeyboardComponent::modifierKeysChanged(const juce::ModifierKeys &modifiers)
{
    if (!shiftHeld && modifiers.isShiftDown())
    {
        shiftHeld = true;
    }

    if (shiftHeld && !modifiers.isShiftDown())
    {
        shiftHeld = false;
    }

    if (!altHeld && modifiers.isAltDown())
    {
       altHeld = true;
    }

    else if (altHeld && !modifiers.isAltDown())
    {
       altHeld = false;
    }

    if (!ctrlHeld && modifiers.isCtrlDown())
    {
       ctrlHeld = true;
    }

    else if (ctrlHeld && !modifiers.isCtrlDown())
    {
       ctrlHeld = false;
    }

    LumatoneKeyboardComponent::modifierKeysChanged(modifiers);
}

void LumatoneEditorKeyboardComponent::noteOnInternal(int midiChannel, int midiNote, juce::uint8 velocity)
{
    // TODO - make optional virtual mapping to make all keys unique, just for software interaction?
    auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);

    // If a key results in multiple virtual keys (shares MIDI data) then reset selection, but "hold down" command for subsequent keys
    bool multiSelect = mappedKeyCoords.size() > 1;
    ModifierKeys keys;

    for (auto coord : mappedKeyCoords)
    {
        if (LumatoneEditorState::getMappingData()->isKeyCoordValid(coord))
        {
            // TODO - solve issue for auto increment notes
            LumatoneEditorState::Controller::DoEditKeyDownAction(coord, keys);
            updateKeyState(coord.boardIndex, coord.keyIndex, true);
        }

        if (multiSelect)
            keys = keys.withFlags(juce::ModifierKeys::commandModifier);
    }
}

void LumatoneEditorKeyboardComponent::deselectColour()
{

}

juce::Colour LumatoneEditorKeyboardComponent::getSelectedColour()
{
    return selectedColour;
}
