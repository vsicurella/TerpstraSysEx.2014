/*
  ==============================================================================

    EditorControlActions.h
    Created: 6 June 2024
    Author:  Vito

  ==============================================================================
*/

#ifndef LUMATONE_EDITOR_CONTROL_ACTIONS_H
#define LUMATONE_EDITOR_CONTROL_ACTIONS_H

#include "../data/LumatoneEditorState.h"
#include "../lumatone_editor_library/actions/lumatone_action.h"
#include "../data/LumatoneEditSelectionState.h"

// Records action of setting control values in the editor (before assign action)
class SetKeySettingsAction : public LumatoneEditorState
                           , private LumatoneEditorState::Controller
                           , public LumatoneAction
{
public:

    SetKeySettingsAction(const LumatoneEditorState& stateIn
                       , bool setKeyColourIn
                       , bool setKeyTypeIn
                       , bool setKeyNoteIn
                       , bool setKeyChannelIn
                       , bool setCCFaderDefaultIn
                       , juce::Colour colourIn = juce::Colour()
                       , LumatoneKeyType typeIn = LumatoneKeyType::noteOnNoteOff
                       , int noteIn = 0
                       , int channelIn = 0
                       , bool ccFaderDefaultIn = false,
                        bool addToSettings = false);

    SetKeySettingsAction(const LumatoneEditorState& stateIn, const LumatoneKeyPropertyData& settingsData);

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(SetKeySettingsAction); }

    void addToEditAassignment();
    void setEditAssignment();

public:

    static SetKeySettingsAction* NewSetAssignColourAction(LumatoneEditorState&  stateIn,    juce::Colour colourIn, bool addToSettings = true);
    static SetKeySettingsAction* NewSetAssignKeyTypeAction(LumatoneEditorState& stateIn,    LumatoneKeyType typeIn, bool addToSettings = true);
    static SetKeySettingsAction* NewSetAssignKeyNoteAction(LumatoneEditorState& stateIn,    int noteIn, bool addToSettings = true);
    static SetKeySettingsAction* NewSetAssignKeyChannelAction(LumatoneEditorState& stateIn, int channelIn, bool addToSettings = true);
    static SetKeySettingsAction* NewSetAssignCCFaderAction(LumatoneEditorState& stateIn,    bool faderDefaultIn, bool addToSettings = true);

private:

    LumatoneKeyPropertyData previousData;
    LumatoneKeyPropertyData newEditData;

public:

    bool add = false;

};

class ApplyAssignmentsToSelectionAction : public LumatoneEditorState
                                        , private LumatoneEditorState::Controller
                                        , public LumatoneAction
{
public:

    ApplyAssignmentsToSelectionAction(const LumatoneEditorState& stateIn
                                    , LumatoneKeyPropertyData assignData
                                    , const juce::Array<MappedLumatoneKey>& keySelectionIn);

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(ApplyAssignmentsToSelectionAction); }

private:

    void applyData(LumatoneKeyPropertyData& dataToApply);

private:
    juce::Array<MappedLumatoneKey>    previousData;
    LumatoneKeyPropertyData  newData;

    juce::Array<MappedLumatoneKey> keySelection;

};

class SetCurrentFileAction : public LumatoneEditorState
                                , private LumatoneEditorState::Controller
                                , public LumatoneAction
{
public:

    SetCurrentFileAction(const LumatoneEditorState& stateIn, juce::File file);
    ~SetCurrentFileAction() override;
    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(SetCurrentFileAction); }

private:

    juce::File previousFile;
    juce::File newFile;
    std::shared_ptr<LumatoneLayout> previousMappingData;
    juce::Array<MappedLumatoneKey> previousKeySelection;
};

class SetEditMode : public LumatoneEditorState
                    , private LumatoneEditorState::Controller
                    , public LumatoneAction
{
public:

    SetEditMode(const LumatoneEditorState& stateIn, LumatoneEditor::MouseMode mouseMode, bool incrementNotes=false, int incrementChannelsPerNotes = 0);
    ~SetEditMode() override {}

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(SetEditMode); }

public:

    static SetEditMode* SetSelectMode(const LumatoneEditorState& stateIn) { return new SetEditMode(stateIn, LumatoneEditor::MouseMode::SELECT); }
    static SetEditMode* SetAssignMode(const LumatoneEditorState& stateIn, bool autoIncrNotes, int incrChannelsPerNotes=0)
    {
        return new SetEditMode(stateIn, LumatoneEditor::MouseMode::ASSIGN, autoIncrNotes, incrChannelsPerNotes);
    }

private:

    LumatoneEditor::MouseMode mouseMode;
    bool incrementNotesState = false;
    int incrementChannelsState = 0;

    LumatoneEditor::MouseMode lastMouseMode;
    bool lastIncrementNotesState = false;
    int lastIncrementChannelsState = 0;
};

#endif // LUMATONE_EDITOR_CONTROL_ACTIONS_H
