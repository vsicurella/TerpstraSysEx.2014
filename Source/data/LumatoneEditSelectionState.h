/*
  ==============================================================================

    LumatoneEditSelectionState.h
    Created: 6 June 2024
    Author:  Vito

    Holds the key property values that are active in a key assignment controller

  ==============================================================================
*/


#ifndef LUMATONE_EDIT_SELECTION_STATE_H
#define LUMATONE_EDIT_SELECTION_STATE_H

#include "../lumatone_editor_library/data/state_base.h"
#include "../lumatone_editor_library/data/lumatone_key.h"

struct LumatoneKeySelectionData
{
    juce::Array<juce::Colour> colours;
    juce::Array<LumatoneKeyType> types;
    juce::Array<int> notes;
    juce::Array<int> channels;
    juce::Array<bool> ccFlags;

    bool hasColours() const { return colours.size() > 0; }
    bool hasOneColour() const { return colours.size() == 1; }
    bool hasMultipleColours() const { return colours.size()  > 1; }

    bool hasTypes() const { return types.size() > 0; }
    bool hasOneType() const { return types.size() == 1; }
    bool hasMultipleTypes() const { return types.size()  > 1; }

    bool hasNotes() const { return notes.size() > 0; }
    bool hasOneNote() const { return notes.size() == 1; }
    bool hasMultipleNotes() const { return notes.size()  > 1; }

    bool hasChannels() const { return channels.size() > 0; }
    bool hasOneChannel() const { return channels.size() == 1; }
    bool hasMultipleChannels() const { return channels.size()  > 1; }

    bool hasCcFlags() const { return ccFlags.size() > 0; }
    bool hasOneCcFlag() const { return ccFlags.size() == 1; }
    bool hasMultipleCcFlags() const { return ccFlags.size()  > 1; }
};

class LumatoneEditSelectionState : public LumatoneStateBase
{
public:

    LumatoneEditSelectionState(juce::String nameIn, juce::ValueTree parentStateIn);
    LumatoneEditSelectionState(juce::String nameIn,  juce::ValueTree parentStateIn, const LumatoneEditSelectionState& stateToCopy);

    virtual ~LumatoneEditSelectionState() override { };

    LumatoneKeyPropertyData getData() const { return data; }
    const LumatoneKeyPropertyData& readData() const { return data; }

    void setKeyColour(bool set, juce::Colour colourIn);
    void setKeyType(bool set, LumatoneKeyType typeIn);
    void setKeyNote(bool set, int noteIn);
    void setKeyChannel(bool set, int channelIn);
    void setCCFader(bool set, bool ccFaderDefaultIn);

    void clear();

public:

    // Get lists of all selected properties
    static LumatoneKeySelectionData findSelectionProperties(const juce::Array<MappedLumatoneKey>& selection);

    // Get properties shared with all selected keys
    static LumatoneKeyPropertyData findSharedSelectionProperties(const juce::Array<MappedLumatoneKey>& selection);
    static LumatoneKeyPropertyData findSharedSelectionProperties(const LumatoneKeySelectionData& properties);

protected:

    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    void loadPropertiesFile(juce::PropertiesFile*) override { };

private:

    LumatoneKeyPropertyData data;
};

namespace LumatoneEditSelectionProperty
{
    static const juce::Identifier AssignKeyEdits = juce::Identifier("AssignKeyEdits");
    static const juce::Identifier AssignKeyColour = juce::Identifier("AssignKeyColour");
    static const juce::Identifier AssignKeyType = juce::Identifier("AssignKeyType");
    static const juce::Identifier AssignKeyNote = juce::Identifier("AssignKeyNote");
    static const juce::Identifier AssignKeyChannel = juce::Identifier("AssignKeyChannel");
    static const juce::Identifier AssignKeyCCFader = juce::Identifier("AssignKeyCCFader");
}


#endif // LUMATONE_EDIT_SELECTION_STATE_H
