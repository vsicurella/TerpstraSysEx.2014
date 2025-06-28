#include "EditorControlActions.h"


SetKeySettingsAction::SetKeySettingsAction(const LumatoneEditorState & stateIn,
                                           bool setKeyColourIn,
                                           bool setKeyTypeIn,
                                           bool setKeyNoteIn,
                                           bool setKeyChannelIn,
                                           bool setCCFaderDefaultIn,
                                           juce::Colour colourIn,
                                           LumatoneKeyType typeIn,
                                           int noteIn,
                                           int channelIn,
                                           bool ccFaderDefaultIn,
                                            bool addToSettings)
	: LumatoneEditorState("SetKeySettingsAction", stateIn)
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    , LumatoneAction(this, "SetKeySettingsAction")
    , add(addToSettings)
    // , newEditData("SetKeySettingsActionData")
{
    previousData = stateIn.getEditSelectionData();

    newEditData.useColour = setKeyColourIn;
    newEditData.colour = colourIn;

    newEditData.useType = setKeyTypeIn;
    newEditData.type = typeIn;

    newEditData.useNote = setKeyNoteIn;
    newEditData.note = noteIn;

    newEditData.useChannel = setKeyChannelIn;
    newEditData.channel = channelIn;

    newEditData.useCCFaderDefault = setCCFaderDefaultIn;
    newEditData.ccFaderDefault = ccFaderDefaultIn;
}

SetKeySettingsAction::SetKeySettingsAction(const LumatoneEditorState &stateIn, const LumatoneKeyPropertyData &settingsData)
    : SetKeySettingsAction(stateIn,
        settingsData.useColour, settingsData.useType, settingsData.useNote, settingsData.useChannel, settingsData.useCCFaderDefault,
        settingsData.colour, settingsData.type, settingsData.note, settingsData.channel, settingsData.ccFaderDefault)
{

}

bool SetKeySettingsAction::perform()
{
    if (add)
        addToEditAassignment();
    else
        setEditAssignment();
    return true;
}

bool SetKeySettingsAction::undo()
{
    setAssignKeyColour(previousData.useColour, previousData.colour);
    setAssignKeyType(previousData.useType, previousData.type);
    setAssignKeyNote(previousData.useNote, previousData.note);
    setAssignKeyChannel(previousData.useChannel, previousData.channel);
    setAssignCCFader(previousData.useCCFaderDefault, previousData.ccFaderDefault);
    return true;
}


void SetKeySettingsAction::addToEditAassignment()
{
    if (newEditData.useColour)
        setAssignKeyColour(newEditData.useColour, newEditData.colour);
    if (newEditData.useType)
        setAssignKeyType(newEditData.useType, newEditData.type);
    if (newEditData.useNote)
        setAssignKeyNote(newEditData.useNote, newEditData.note);
    if (newEditData.useChannel)
        setAssignKeyChannel(newEditData.useChannel, newEditData.channel);
    if (newEditData.useCCFaderDefault)
        setAssignCCFader(newEditData.useCCFaderDefault, newEditData.ccFaderDefault);
}

void SetKeySettingsAction::setEditAssignment()
{
    editSelectionState.clear();
    addToEditAassignment();
}

SetKeySettingsAction *SetKeySettingsAction::NewSetAssignColourAction(LumatoneEditorState &stateIn, juce::Colour colourIn, bool addToSettings)
{
    bool valid = colourIn.isOpaque();
    SetKeySettingsAction* action = new SetKeySettingsAction(stateIn, valid, false, false, false, false, colourIn);
    action->add = addToSettings;
    return action;
}

SetKeySettingsAction *SetKeySettingsAction::NewSetAssignKeyTypeAction(LumatoneEditorState &stateIn, LumatoneKeyType typeIn, bool addToSettings)
{
    bool valid = typeIn > LumatoneKeyType::disabledDefault && typeIn <= LumatoneKeyType::disabled;
    SetKeySettingsAction* action = new SetKeySettingsAction(stateIn, false, valid, false, false, false, juce::Colour(), typeIn);
    action->add = addToSettings;
    return action;
}

SetKeySettingsAction *SetKeySettingsAction::NewSetAssignKeyNoteAction(LumatoneEditorState &stateIn, int noteIn, bool addToSettings)
{
    bool valid = noteIn >= 0 && noteIn < 128;
    SetKeySettingsAction* action = new SetKeySettingsAction(stateIn, false, false, valid, false, false, juce::Colour(), LumatoneKeyType(), noteIn);
    action->add = addToSettings;
    return action;
}

SetKeySettingsAction *SetKeySettingsAction::NewSetAssignKeyChannelAction(LumatoneEditorState &stateIn, int channelIn, bool addToSettings)
{
    bool valid = channelIn > 0 && channelIn <= 16;
    SetKeySettingsAction* action = new SetKeySettingsAction(stateIn, false, false, false, valid, false, juce::Colour(), LumatoneKeyType(), 0, channelIn);
    action->add = addToSettings;
    return action;
}

SetKeySettingsAction *SetKeySettingsAction::NewSetAssignCCFaderAction(LumatoneEditorState &stateIn, bool faderDefaultIn, bool addToSettings)
{
    SetKeySettingsAction* action = new SetKeySettingsAction(stateIn, false, false, false, false, true, juce::Colour(), LumatoneKeyType(), 0, 0, faderDefaultIn);
    action->add = addToSettings;
    return action;
}

ApplyAssignmentsToSelectionAction::ApplyAssignmentsToSelectionAction(const LumatoneEditorState& stateIn
                                                                   , LumatoneKeyPropertyData assignData
                                                                   , const juce::Array<MappedLumatoneKey> &keySelectionIn)
	: LumatoneEditorState("ApplyAssignmentsToSelectionAction", stateIn)
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    , LumatoneAction(this, "ApplyAssignmentsToSelectionAction")
{

    newData = assignData;

    for (const MappedLumatoneKey& key : keySelectionIn)
    {
        keySelection.add(key);
        previousData.add(key);
    }
}

bool ApplyAssignmentsToSelectionAction::perform()
{
    juce::Array<MappedLumatoneKey> updatedKeys;

    for (const MappedLumatoneKey key : keySelection)
    {
        MappedLumatoneKey keyUpdate = key;
        if (newData.useColour)
            keyUpdate.setColour(newData.colour);
        if (newData.useType)
            keyUpdate.setKeyType(newData.type);
        if (newData.useNote)
            keyUpdate.setNoteOrCC(newData.note);
        if (newData.useChannel)
            keyUpdate.setChannelNumber(newData.channel);
        if (newData.ccFaderDefault)
            keyUpdate.setDefaultCCFader(newData.ccFaderDefault);

        LumatoneState::setKey((const LumatoneKey&) keyUpdate, keyUpdate.boardIndex + 1, keyUpdate.keyIndex);
        updatedKeys.add(keyUpdate);
    }

    bool useBuffer = updatedKeys.size() > 48;

    if (!newData.useColour)
    {
        // todo param-only update (no colour)
        sendSelectionParam(updatedKeys, true, useBuffer);
    }
    else if (newData.useType || newData.useNote || newData.useChannel || newData.ccFaderDefault)
    {
        sendSelectionParam(updatedKeys, true, useBuffer);
    }
    else
    {
        sendSelectionColours(updatedKeys, true, useBuffer);
    }

    updatedSelectedKeys();

    return true;
}

bool ApplyAssignmentsToSelectionAction::undo()
{
    // for (const MappedLumatoneKey key : previousData)
    // {
    //     setKey((const LumatoneKey&) key, key.boardIndex + 1, key.keyIndex);
    // }

    sendSelectionParam(previousData);
    updatedSelectedKeys();

    return true;
}

SetCurrentFileAction::SetCurrentFileAction(const LumatoneEditorState& stateIn, juce::File file)
    : LumatoneEditorState("SetCurrentFileAction", stateIn)
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    , LumatoneAction(this, "SetCurrentFileAction")
    , newFile(file)
{
    previousFile = getCurrentFile();
    previousMappingData = shareMappingData();
    previousKeySelection.addArray(*getSelectedKeys());
}

SetCurrentFileAction::~SetCurrentFileAction()
{
    previousMappingData = nullptr;
}

bool SetCurrentFileAction::perform()
{
    setCurrentFile(newFile);
    if (resetToCurrentFile())
    {
        // Clear undo history
        // undoManager.clearUndoHistory();

        clearSelectedKeys();
        return true;
    }

    return false;
}

bool SetCurrentFileAction::undo()
{
    if (previousFile.existsAsFile())
        setCurrentFile(previousFile);
    setCompleteConfig(*previousMappingData);
    setSelectedKeys(previousKeySelection);
    return true;
}

SetEditMode::SetEditMode(const LumatoneEditorState& stateIn, LumatoneEditor::MouseMode newMouseMode, bool incrementNotes, int incrementChannelsPerNotes)
    : LumatoneEditorState(stateIn)
    , LumatoneEditorState::Controller(static_cast<LumatoneEditorState&>(*this))
    , LumatoneAction(this, "SetEditMode")
    , mouseMode(newMouseMode), incrementNotesState(incrementNotes), incrementChannelsState(incrementChannelsPerNotes)
{
    lastMouseMode = getMouseMode();
    lastIncrementNotesState = isIncrementingNotes();
    lastIncrementChannelsState = isIncrementingChannelsAfterNumNotes();
}

bool SetEditMode::perform()
{
    setMouseMode(mouseMode);
    setIncrementNotes(incrementNotesState);

    if (incrementChannelsState > 0)
        setChannelsIncrementPerNotes(incrementChannelsState);

    return true;
}

bool SetEditMode::undo()
{
    setMouseMode(lastMouseMode);
    setIncrementNotes(lastIncrementNotesState);

    if (incrementChannelsState > 0 && lastIncrementChannelsState > 0)
        setChannelsIncrementPerNotes(lastIncrementChannelsState);

    return true;
}
