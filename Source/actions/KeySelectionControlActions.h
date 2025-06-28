/*
  ==============================================================================

    KeySelectionControlActions.h
    Created: 15 June 2024
    Author:  Vito

  ==============================================================================
*/

#ifndef LUMATONE_SELECTION_CONTROL_ACTIONS_H
#define LUMATONE_SELECTION_CONTROL_ACTIONS_H

#include "../data/LumatoneEditorState.h"
#include "../lumatone_editor_library/actions/lumatone_action.h"
#include "../data/LumatoneEditSelectionState.h"
class AddOrRemoveKeySelectionAction : public LumatoneEditorState
                                    , private LumatoneEditorState::Controller
                                    , public LumatoneAction
{
public:
    AddOrRemoveKeySelectionAction(const LumatoneEditorState& stateIn, int keyNum, bool remove = false);
    ~AddOrRemoveKeySelectionAction() override;

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(AddOrRemoveKeySelectionAction); }

private:

    int keyNum;
    bool remove;
};

class SetKeySelectionAction : public LumatoneEditorState
                            , private LumatoneEditorState::Controller
                            , public LumatoneAction
{
public:
    SetKeySelectionAction(const LumatoneEditorState& stateIn, juce::Array<MappedLumatoneKey> newSelection);
    ~SetKeySelectionAction() override;

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(SetKeySelectionAction); }

public:

    static SetKeySelectionAction* NewSetKeySelectionActionByCoords(const LumatoneEditorState& stateIn, juce::Array<LumatoneKeyCoord> keyCoords);

private:

    juce::Array<MappedLumatoneKey> previousSelection;
    juce::Array<MappedLumatoneKey> newSelection;

    juce::Colour previousSelectedColour;
};



#endif // LUMATONE_SELECTION_CONTROL_ACTIONS_H

static juce::Colour getColourFromSelectedKeys(const juce::Array<MappedLumatoneKey> *selectedKeys);
static LumatoneKeyType getTypeFromSelectedKeys(const juce::Array<MappedLumatoneKey> *selectedKeys);
static int getNoteFromSelectedKeys(const juce::Array<MappedLumatoneKey> *selectedKeys);
static int getChannelFromSelectedKeys(const juce::Array<MappedLumatoneKey> *selectedKeys);
