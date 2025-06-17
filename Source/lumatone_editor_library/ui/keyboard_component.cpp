/*
  ==============================================================================

    keyboard_component.cpp
    Created: 24 Jul 2023 9:52:28pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "keyboard_component.h"

#include "../color/colour_model.h"
#include "../device/lumatone_controller.h"

LumatoneKeyboardComponent::LumatoneKeyboardComponent(const LumatoneApplicationState& stateIn)
    : LumatoneApplicationState("LumatoneKeyboardComponent", stateIn)
    , lumatoneRender(stateIn)
{
    renderMode = LumatoneComponentRenderMode::GraphicInteractive;
    resetOctaveSize(false);

    addMouseListener(this, this);
    addKeyListener(this);
    setWantsKeyboardFocus(true);

    layoutChanged(*getMappingData());
    updateSelectedKeys(*getSelectedKeys());

    addEditorListener(this);
    addMidiListener(this);
}

LumatoneKeyboardComponent::~LumatoneKeyboardComponent()
{
    removeMidiListener(this);
    removeEditorListener(this);
}

void LumatoneKeyboardComponent::paint (juce::Graphics& g)
{
    switch (renderMode)
    {
    case LumatoneComponentRenderMode::Graphic:
    case LumatoneComponentRenderMode::GraphicInteractive:
        g.drawImageAt(lumatoneGraphic, lumatoneBounds.getX(), lumatoneBounds.getY());
        break;

    case LumatoneComponentRenderMode::MaxRes:
        g.drawImageAt(currentRender, lumatoneBounds.getX(), lumatoneBounds.getY());
        break;
    }

    // Draw a line under the selected sub board
    // if (currentSetSelection >= 0 && currentSetSelection < state.getNumBoards())
    // {
    //     juce::Path selectionMarkPath;
    //     selectionMarkPath.startNewSubPath(octaveBoards[currentSetSelection]->leftPos, octaveLineY);
    //     selectionMarkPath.lineTo(octaveBoards[currentSetSelection]->rightPos, octaveLineY);

    //     juce::Colour lineColour = juce::Colours::white;// findColour(LumatoneKeyEdit::outlineColourId);
    //     g.setColour(lineColour);
    //     g.strokePath(selectionMarkPath, juce::PathStrokeType(1.0f));
    // }
}

void LumatoneKeyboardComponent::resized()
{
    currentWidth = getWidth();
    currentHeight = juce::roundToInt((float)getWidth() / imageAspect);

    if (currentHeight > getHeight())
    {
        currentHeight = getHeight();
        currentWidth = juce::roundToInt((float)currentHeight * imageAspect);
    }

    // Prepare position helpers for graphics
    lumatoneBounds.setBounds(juce::roundToInt((getWidth() - currentWidth) * 0.5f),
                             juce::roundToInt((getHeight() - currentHeight) * 0.5f),
                             currentWidth,
                             currentHeight);

    octaveLineY = lumatoneBounds.getBottom() + juce::roundToInt(getHeight() * octaveLineYRatio);

    keyWidth = juce::roundToInt(lumatoneBounds.getWidth() * keyW);
    keyHeight = juce::roundToInt(lumatoneBounds.getHeight() * keyH);

    switch (renderMode)
    {
        case LumatoneComponentRenderMode::Graphic:
        case LumatoneComponentRenderMode::GraphicInteractive:
            lumatoneGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::LumatoneGraphic, lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
            keyShapeGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::KeyShape, keyWidth, keyHeight);
            break;
        case LumatoneComponentRenderMode::MaxRes:
            currentRender = lumatoneRender.getResizedRender(lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
            break;
    }

    keyShadowGraphic = lumatoneRender.getResizedAsset(LumatoneAssets::ID::KeyShadow, keyWidth, keyHeight, true);

    int octaveIndex = 0;
    int octaveX = keyCentres[0].getX() * lumatoneBounds.getWidth() + lumatoneBounds.getX();
    octaveBoards[octaveIndex]->leftPos = octaveX - keyWidth * 0.5;

    const int octaveBoardSize = getOctaveBoardSize();

    for (int keyIndex = 0; keyIndex < keyCentres.size(); keyIndex++)
    {
        int keyOctaveIndex = keyIndex % octaveBoardSize;

        juce::Point<int> keyPos = juce::Point<int>(
            juce::roundToInt(keyCentres[keyIndex].x * lumatoneBounds.getWidth() + lumatoneBounds.getX() - keyWidth * 0.5f),
            juce::roundToInt(keyCentres[keyIndex].y * lumatoneBounds.getHeight() + lumatoneBounds.getY() - keyHeight * 0.5f)
        );

        auto key = octaveBoards[octaveIndex]->keyMiniDisplay[keyOctaveIndex];
        key->setKeyGraphics(keyShapeGraphic, keyShadowGraphic);
        key->setBounds(keyPos.x, keyPos.y, keyWidth, keyHeight);

        if (keyOctaveIndex + 1 == octaveBoardSize)
        {
            octaveBoards[octaveIndex]->rightPos = key->getRight();
            octaveIndex++;

            if (octaveIndex < getNumBoards())
                octaveBoards[octaveIndex]->leftPos = key->getX();
        }
    }
}

void LumatoneKeyboardComponent::resetOctaveSize(bool resetState)
{
    const int octaveBoardSize = getOctaveBoardSize();
    if (currentOctaveSize != octaveBoardSize)
    {
        octaveBoards.clear();

        for (int subBoardIndex = 0; subBoardIndex < getNumBoards(); subBoardIndex++)
        {
            OctaveBoard* board = octaveBoards.add(new OctaveBoard());

            for (int keyIndex = 0; keyIndex < octaveBoardSize; keyIndex++)
            {
                const LumatoneKey& keyData = getKey(subBoardIndex, keyIndex);
                auto key = board->keyMiniDisplay.add(new LumatoneKeyDisplay(subBoardIndex, keyIndex, keyData));
                key->setRenderMode(renderMode);
                addAndMakeVisible(key);
            }
        }

        currentOctaveSize = octaveBoardSize;
    }

    lumatoneRender.resetOctaveSize();
    keyCentres = lumatoneRender.getKeyCentres();

    if (resetState)
        resetLayoutState();
}

void LumatoneKeyboardComponent::setRenderMode(LumatoneComponentRenderMode modeIn)
{
    renderMode = modeIn;
    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            auto key = board->keyMiniDisplay[keyIndex];
            key->setRenderMode(renderMode);
        }
    }

    mappingUpdateCallback();
}

void LumatoneKeyboardComponent::setUiMode(LumatoneKeyboardComponent::UiMode modeIn)
{
    if (uiMode == LumatoneKeyboardComponent::UiMode::Perform && uiMode != modeIn)
    {
        clearHeldNotes();
    }

    uiMode = modeIn;
}

void LumatoneKeyboardComponent::setShowKeyProperties(bool showProps)
{
    if (showMidiInfo != showProps)
    {
        showMidiInfo = showProps;
        for (auto& board : octaveBoards)
        {
            for (auto& key : board->keyMiniDisplay)
            {
                key->setShowMidiInfo(showProps);
            }
        }
    }

}

juce::Rectangle<int> LumatoneKeyboardComponent::getLocalGraphicBounds() const
{
    return lumatoneBounds;
}

void LumatoneKeyboardComponent::layoutChanged(const LumatoneLayout& mappingData)
{
    for (int boardIndex = 0; boardIndex < octaveBoards.size(); boardIndex++)
    {
        auto board = octaveBoards[boardIndex];

        for (int keyIndex = 0; keyIndex < board->keyMiniDisplay.size(); keyIndex++)
        {
            const LumatoneKey& keyData = mappingData.getKey(boardIndex, keyIndex);
            keyUpdateCallback(boardIndex, keyIndex, keyData, false);
        }
    }

    resetLayoutState(&mappingData);
}

void LumatoneKeyboardComponent::boardChanged(const LumatoneBoard& boardData)
{
    int boardIndex = boardData.getBoardIndex();

    for (int keyIndex = 0; keyIndex < octaveBoards[boardIndex]->keyMiniDisplay.size(); keyIndex++)
    {
        const LumatoneKey& keyData = boardData.getKey(keyIndex);
        keyUpdateCallback(boardIndex, keyIndex, keyData, false);
    }

    resetLayoutState();
}

void LumatoneKeyboardComponent::contextChanged(LumatoneContext *newOrEmptyContext)
{
    // LumatoneLayout layout = (newOrEmptyContext == nullptr)
    //     ? *getMappingData()
    //     : *newOrEmptyContext;

    // completeMappingLoaded(layout);
}

void LumatoneKeyboardComponent::keyChanged(int boardIndex, int keyIndex, const LumatoneKey& lumatoneKey)
{
    keyUpdateCallback(boardIndex, keyIndex, lumatoneKey);
}

// void LumatoneKeyboardComponent::keyConfigChanged(int boardIndex, int keyIndex, const LumatoneKey& keyData)
// {
//     keyUpdateCallback(boardIndex, keyIndex, keyData);
// }

// void LumatoneKeyboardComponent::keyColourChanged(int boardIndex, int keyIndex, juce::Colour keyColour)
// {
//     keyUpdateCallback(boardIndex, keyIndex, getKey(boardIndex, keyIndex));
// }

void LumatoneKeyboardComponent::keySetChanged(juce::Array<MappedLumatoneKey> selection)
{

    // auto paintKey = renderMode != LumatoneComponentRenderMode::MaxRes;
    // if (renderMode == LumatoneComponentRenderMode::MaxRes)
    //     rerender();

    for (const MappedLumatoneKey& key : selection)
    {
        keyUpdateCallback(key.boardIndex, key.keyIndex, key);
    }
}

void LumatoneKeyboardComponent::selectionChanged()
{
    updateSelectedKeys(*getSelectedKeys());
}

void LumatoneKeyboardComponent::keyUpdateCallback(int boardIndex, int keyIndex, const LumatoneKey& newKey, bool doRepaint)
{
    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];

    applyKeyUpdates(boardIndex, keyIndex, newKey);

    if (!doRepaint)
        return;

    if (!key->configIsEqual(newKey))
    {
        resetLayoutState();
    }
    else if (!key->isColour(newKey.getColour()))
    {
        if (renderMode == LumatoneComponentRenderMode::MaxRes)
            rerender();
        else
        {
            key->repaint();
        }
    }
    else
        key->repaint(); //kludge
}

void LumatoneKeyboardComponent::mappingUpdateCallback()
{
    if (renderMode == LumatoneComponentRenderMode::MaxRes)
        lumatoneRender.render();

    if (currentWidth == 0 || currentHeight == 0)
        return;

    // resized();
    repaint(lumatoneBounds);
}

void LumatoneKeyboardComponent::rerender()
{
    lumatoneRender.render();
    currentRender = lumatoneRender.getResizedRender(lumatoneBounds.getWidth(), lumatoneBounds.getHeight());
    repaint(lumatoneBounds);
}

void LumatoneKeyboardComponent::updateSelectedKeys(const juce::Array<MappedLumatoneKey>& newSelection)
{
    for (const MappedLumatoneKey& mappedKey : lastKeySelection)
    {
        if (mappedKey.boardIndex < 0)
        {
            DBG("ERR: bad board index!");
            break;
        }

        if (mappedKey.keyIndex < 0)
        {
            DBG("ERR: bad key index!");
            break;
        }

        const OctaveBoard* board = octaveBoards.getUnchecked(mappedKey.boardIndex);
        board->keyMiniDisplay.getUnchecked(mappedKey.keyIndex)->setSelected(false);
        board->keyMiniDisplay.getUnchecked(mappedKey.keyIndex)->setShowMidiInfo(showMidiInfo);
    }

    for (const MappedLumatoneKey& mappedKey : newSelection)
    {
        if (mappedKey.boardIndex < 0)
        {
            DBG("ERR: bad board index!");
            break;
        }

        if (mappedKey.keyIndex < 0)
        {
            DBG("ERR: bad key index!");
            break;
        }

        const OctaveBoard* board = octaveBoards.getUnchecked(mappedKey.boardIndex);
        board->keyMiniDisplay.getUnchecked(mappedKey.keyIndex)->setSelected(true);
        board->keyMiniDisplay.getUnchecked(mappedKey.keyIndex)->setShowMidiInfo(true);
    }

    lastKeySelection.clearQuick();
    lastKeySelection.addArray(newSelection);
}

void LumatoneKeyboardComponent::applyKeyUpdates(int boardIndex, int keyIndex, const LumatoneKey& keyData)
{
    auto modelColour = getColourModel()->getModelColour(keyData.getColour());
    // DBG("Adj. " + keyData.getColour().toDisplayString(true) + " to " + modelColour.toDisplayString(true));
    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
    key->setLumatoneKey(keyData, modelColour);
}

void LumatoneKeyboardComponent::resetLayoutState(const LumatoneLayout* optionalLayout)
{
    const LumatoneLayout* layout = optionalLayout;
    if (layout == nullptr)
        layout = getMappingData();

    lumatoneMidiMap.render(*layout);
    LumatoneMidiState::reset();
    mappingUpdateCallback();
}

void LumatoneKeyboardComponent::clearHeldNotes()
{
    for (auto key : keysOn)
    {
        if (key)
        {
            key->noteOff();
            key->clearUiState();
        }
    }

    keysOn.clear();
}

void LumatoneKeyboardComponent::sustainStarted()
{

}

void LumatoneKeyboardComponent::sustainEnded()
{
    clearHeldNotes();
}

void LumatoneKeyboardComponent::lumatoneKeyDown(int boardIndex, int keyIndex)
{
    auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
    if (getMappingData()->isKeyCoordValid(coord))
    {
        juce::uint8 velocity = 0x70;
        if (ctrlHeld)
            velocity = 0x30;

        keyDownInternal(boardIndex, keyIndex, velocity);
    }
}

void LumatoneKeyboardComponent::lumatoneKeyUp(int boardIndex, int keyIndex)
{
    auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
    if (getMappingData()->isKeyCoordValid(coord))
    {
        keyUpInternal(boardIndex, keyIndex);
    }
}

void LumatoneKeyboardComponent::updateKeyState(int boardIndex, int keyIndex, bool keyIsDown)
{
    auto keyComponent = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];

    jassert(keyComponent != nullptr);
    if (keyComponent != nullptr)
    {
        if (keyIsDown)
            keyComponent->noteOn();
        else
            keyComponent->noteOff();
    }
}

void LumatoneKeyboardComponent::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property)
{
    LumatoneApplicationState::handleStatePropertyChange(stateIn, property);

    if (property == LumatoneApplicationProperty::ColourMode)
    {
        juce::String mode = stateIn[property].toString();
        for (int b = 0; b < getNumBoards(); b++)
        {
            OctaveBoard* board = octaveBoards.getUnchecked(b);
            for (int k = 0; k < getOctaveBoardSize(); k++)
            {
                LumatoneKey key = getKey(b, k);
                juce::Colour colour = key.getColour();
                if (mode == "None" || mode == "ModelAdjusted")
                {
                    colour = getColourModel()->getModelColour(colour);
                }

                board->keyMiniDisplay.getUnchecked(k)->setLumatoneKey(key, colour);
            }
        }
    }
}

LumatoneKeyDisplay* LumatoneKeyboardComponent::getKeyFromMouseEvent(const juce::MouseEvent& e)
{
    juce::Point<float> position = e.position;

    if (e.eventComponent != this)
    {
        position = e.getEventRelativeTo(this).position;
    }

    auto child = getComponentAt(position);
    LumatoneKeyDisplay* key = nullptr;
    if (child && child->getParentComponent() == this)
        key = (LumatoneKeyDisplay*)child;

    return key;
}

void LumatoneKeyboardComponent::mouseMove(const juce::MouseEvent& e)
{
    // auto lastOver = keysOverPerMouse[mouseIndex];
    LumatoneKeyCoord keyCoord;
    auto key = getKeyFromMouseEvent(e);

    // if (state.isKeyCoordValid(lastOver))
    // {
    //     auto lastOverForMouse = octaveBoards[lastOver.boardIndex]->keyMiniDisplay[lastOver.keyIndex];
    // }

    mouseMoveInternal(e, key);
}

void LumatoneKeyboardComponent::mouseMoveInternal(const juce::MouseEvent&e, LumatoneKeyDisplay* key)
{
    if (key == nullptr)
        return;
    const int mouseIndex = e.source.getIndex();
    keysOverPerMouse.set(mouseIndex, key->getKeyCoord());
}

void LumatoneKeyboardComponent::mouseDown(const juce::MouseEvent& e)
{
    auto key = getKeyFromMouseEvent(e);
    mouseDownInternal(e, key);
}

void LumatoneKeyboardComponent::mouseDownInternal(const juce::MouseEvent &e, LumatoneKeyDisplay* key)
{
    lumatoneKeyDown(key->getBoardIndex(), key->getKeyIndex());
    lastMouseKeyDown = key;

    keysDownPerMouse.set(e.source.getIndex(), key->getCoord());
    keysOn.addIfNotAlreadyThere(key);
}

void LumatoneKeyboardComponent::mouseUp(const juce::MouseEvent& e)
{
    mouseUpInternal(e, nullptr);
}

void LumatoneKeyboardComponent::mouseUpInternal(const juce::MouseEvent &e, LumatoneKeyDisplay *key)
{
    if (lastMouseKeyDown)
    {
        lumatoneKeyUp(lastMouseKeyDown->getBoardIndex(), lastMouseKeyDown->getKeyIndex());
        keysOn.removeFirstMatchingValue(lastMouseKeyDown);
    }

    keysDownPerMouse.set(e.source.getIndex(), LumatoneKeyCoord());
}

void LumatoneKeyboardComponent::mouseDrag(const juce::MouseEvent& e)
{
    auto key = getKeyFromMouseEvent(e);
    mouseDragInternal(e, key);
}

void LumatoneKeyboardComponent::mouseDragInternal(const juce::MouseEvent &e, LumatoneKeyDisplay *key)
{
    const int mouseIndex = e.source.getIndex();
    auto lastDownCoord = keysDownPerMouse[mouseIndex];

    LumatoneKeyDisplay* mouseKeyLastDown = nullptr;
    if (getMappingData()->isKeyCoordValid(lastDownCoord))
        mouseKeyLastDown = octaveBoards[lastDownCoord.boardIndex]->keyMiniDisplay[lastDownCoord.keyIndex];

    LumatoneKeyCoord keyCoord;
    if (key)
    {
        keyCoord = key->getCoord();
    }

    bool validKey = getMappingData()->isKeyCoordValid(keyCoord);
    bool keyChanged = lastDownCoord != keyCoord;
    bool onNewKey = validKey && (mouseKeyLastDown == nullptr || keyChanged);

    bool setLastNoteOff = !e.mods.isShiftDown() && (onNewKey || !validKey);
    if (mouseKeyLastDown != nullptr && setLastNoteOff)
    {
        keyUpInternal(mouseKeyLastDown->boardIndex, mouseKeyLastDown->keyIndex);
    }

    if (onNewKey)
    {
        keysDownPerMouse.set(mouseIndex, keyCoord);
        keysOverPerMouse.set(mouseIndex, keyCoord);

        juce::uint8 velocity = 0x70;
        if (ctrlHeld)
            velocity = 0x30;

        keyDownInternal(keyCoord.boardIndex, keyCoord.keyIndex, velocity);

        lastMouseKeyOver = key;
        lastMouseKeyDown = key;

        mouseDragInternalOnNewKey(e, key);
    }
    else if (!e.mods.isShiftDown())
    {
        if (validKey)
        {

        }
        else
        {
            keysDownPerMouse.set(mouseIndex, LumatoneKeyCoord());
            keysOverPerMouse.set(mouseIndex, LumatoneKeyCoord());
        }
    }
}


bool LumatoneKeyboardComponent::keyStateChanged(bool isKeyDown)
{
    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey) && upHeld)
    //{
    //    upHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey) && downHeld)
    //{
    //    downHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey) && leftHeld)
    //{
    //    leftHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey) && rightHeld)
    //{
    //    rightHeld = false;
    //    return true;
    //}

    //if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey) && spaceHeld)
    //{
    //    spaceHeld = false;
    //    return true;
    //}

    return false;
}

bool LumatoneKeyboardComponent::keyPressed(const juce::KeyPress& key, Component* originatingComponent)
{
    return false;
}

void LumatoneKeyboardComponent::modifierKeysChanged(const juce::ModifierKeys& modifiers)
{
    //if (!rightMouseHeld && modifiers.isRightButtonDown())
    //{
    //    rightMouseHeld = true;
    //}
    //if (rightMouseHeld && !modifiers.isRightButtonDown())
    //{
    //    rightMouseHeld = false;
    //}

    if (!shiftHeld && modifiers.isShiftDown())
    {
        shiftHeld = true;
    }
    else if (shiftHeld && !modifiers.isShiftDown())
    {
        shiftHeld = false;

        // clearHeldNotes();
    }

    //if (!altHeld && modifiers.isAltDown())
    //{
    //    altHeld = true;
    //    if (uiModeSelected != UIMode::editMode)
    //    {
    //        isolateLastNote();
    //        repaint();
    //    }
    //}

    //else if (altHeld && !modifiers.isAltDown())
    //{
    //    altHeld = false;
    //}

    if (!ctrlHeld && modifiers.isCtrlDown())
    {
       ctrlHeld = true;
    }

    else if (ctrlHeld && !modifiers.isCtrlDown())
    {
       ctrlHeld = false;
    }
}

void LumatoneKeyboardComponent::keyDownInternal(int boardIndex, int keyIndex, juce::uint8 velocity)
{
    if (uiMode == UiMode::None)
        return;

    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
    jassert(key != nullptr);

    if (uiMode == UiMode::Controller)
    {
        auto keyNum = mappingData->keyCoordToKeyNum(boardIndex, keyIndex);
        // listeners.call(&Listener::handleKeyDown, keyNum);
    }
    else switch (key->getType())
    {
        case LumatoneKeyType::noteOnNoteOff:
            //controller->sendKeyNoteOn(boardIndex, keyIndex, velocity);
            noteOn(key->getMidiChannel(), key->getMidiNumber(), velocity);
            break;
        default:
            break;
    }

    updateKeyState(boardIndex, keyIndex, true);
    keysOn.addIfNotAlreadyThere(key);
};

void LumatoneKeyboardComponent::keyUpInternal(int boardIndex, int keyIndex)
{
    if (uiMode == UiMode::None)
        return;

    auto key = octaveBoards[boardIndex]->keyMiniDisplay[keyIndex];
    jassert(key != nullptr);

    if (uiMode == UiMode::Controller)
    {
        // listeners.call(&Listener::handleKeyUp, mappingData->keyCoordToKeyNum(boardIndex, keyIndex));
    }
    else switch (key->getType())
    {
    case LumatoneKeyType::noteOnNoteOff:
        //controller->sendKeyNoteOff(boardIndex, keyIndex);
        noteOff(key->getMidiChannel(), key->getMidiNumber(), 0);
        break;
    default:
        break;
    }

    updateKeyState(boardIndex, keyIndex, false);
    keysOn.removeFirstMatchingValue(key);
}

void LumatoneKeyboardComponent::noteOnInternal(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
    for (auto coord : mappedKeyCoords)
    {
        if (getMappingData()->isKeyCoordValid(coord))
        {
            updateKeyState(coord.boardIndex, coord.keyIndex, true);
        }
    }
}

void LumatoneKeyboardComponent::noteOffInternal(int midiChannel, int midiNote)
{
    auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
    for (auto coord : mappedKeyCoords)
    {
        if (getMappingData()->isKeyCoordValid(coord))
        {
            updateKeyState(coord.boardIndex, coord.keyIndex, false);
        }
    }
}

void LumatoneKeyboardComponent::noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    LumatoneMidiState::noteOnInternal(msg, midiChannel, midiNote, velocity);
    noteOnInternal(midiChannel, midiNote, velocity);
}

void LumatoneKeyboardComponent::noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    LumatoneMidiState::noteOffInternal(msg, midiChannel, midiNote, velocity);
    noteOffInternal(midiChannel, midiNote);
}

// void LumatoneKeyboardComponent::handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity)
// {
//     auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
//     for (auto coord : mappedKeyCoords)
//     {
//         if (getMappingData()->isKeyCoordValid(coord))
//         {
//             updateKeyState(coord.boardIndex, coord.keyIndex, true);
//         }
//     }
// }

// void LumatoneKeyboardComponent::handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote)
// {
//     auto mappedKeyCoords = lumatoneMidiMap.getKeysAssignedToNoteOn(midiChannel, midiNote);
//     for (auto coord : mappedKeyCoords)
//     {
//         if (getMappingData()->isKeyCoordValid(coord))
//         {
//             updateKeyState(coord.boardIndex, coord.keyIndex, false);
//         }
//     }
// }

// void LumatoneKeyboardComponent::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
// {

// }

// void LumatoneKeyboardComponent::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 value)
// {

// }

void LumatoneKeyboardComponent::handleNoteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    noteOnInternal(midiChannel, midiNote, velocity);
}

void LumatoneKeyboardComponent::handleNoteOff(int midiChannel, int midiNote)
{
    noteOffInternal(midiChannel, midiNote);
}
