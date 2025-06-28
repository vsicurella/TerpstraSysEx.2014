/*
  ==============================================================================

    KeyEditorControls.h
    Created: 3 June 2024
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_EDITOR_KEY_EDITOR_CONTROLS_H
#define LUMATONE_EDITOR_KEY_EDITOR_CONTROLS_H

#include "../../data/LumatoneEditorState.h"
#include "../../lumatone_editor_library/listeners/editor_listener.h"
#include "../../lumatone_editor_library/palettes/colour_selection_broadcaster.h"

class ColourSelectionGroup;
class ColourViewComponent;
class LumatoneEditorControl;
class ColourSelectorPanel;
class ColourDropdownSelector;

class KeyEditorControls : public juce::Component
                        , public LumatoneEditorState
                        , private LumatoneEditorState::Controller
                        , public LumatoneEditor::EditorListener
                        , public ColourSelectionListener
                        , public ColourSelectionBroadcaster
                        // , public juce::ChangeListener
{
public:
    KeyEditorControls(const LumatoneEditorState& stateIn);

    virtual ~KeyEditorControls() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void selectionChanged() override;

    // void setSelectionTab(SelectionTabs tab);

private:

    void autoIncrementToggleCallback(bool isToggled);

private:

    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    // Implementation of ColourSelectionListener
    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

    // Implementation of ColourSelectionBroadcaster
    virtual juce::Colour getSelectedColour() override;
    virtual void deselectColour() override;

    // Implementation of ChangeListener
    // void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:

    void colorInputCallback();
    void typeInputCallback();
    void noteInputCallback();
    void channelInputCallback();

    void selectModeCallback();
    void assignModeCallback();

private:
    std::unique_ptr<juce::Label>            lblKeySettings;

    std::unique_ptr<LumatoneEditorControl>  colourInputBox;
    ColourDropdownSelector*                 colourDropdown;
    juce::Colour                            lastSelectedColour;

    std::unique_ptr<LumatoneEditorControl>  keyTypeCombo;
    std::unique_ptr<LumatoneEditorControl>  noteInput;
    std::unique_ptr<LumatoneEditorControl>  channelInput;

    std::unique_ptr<juce::ToggleButton>     autoIncrementToggleButton;
    std::unique_ptr<LumatoneEditorControl>  noteAutoIncrInput;
    std::unique_ptr<LumatoneEditorControl>  channelAutoIncrNoteInput;
    juce::Rectangle<int>                    autoIncrementBounds;
    juce::Path                              autoIncrBorder;

    std::unique_ptr<ColourSelectorPanel>    colourPalettePanel;

    juce::Component* selectionControls = nullptr;

    const juce::StringRef noteInputWidthRef = "_1.6.0._+.+_";

    juce::Path headerPath;
    juce::Path controlPath;

    int contentMarginWidth;
    // const float contentMarginParentW  = 0.02f;

    int contentMarginHeight;
    float controlMarginH        = 0.05f;

    int headerHeight;
    // const float headerH         = 0.19f;

    int labelHeight;
    // const float labelToHeaderH         = 0.5f;
    // const float controlLabelFontScalar = 0.6f;

    int keyControlColumnWidth;
    int keyControlColumnRight;
    float keyControlColumnW     = 0.313f;

    int controlRowHeight;
    int keyControlHeight;
    float keyControlH           = 0.128f;

    int keyControlMarginHeight;
    float keyControlMarginH     = 0.059f;

    int colourColumnX;
    int colourColumnWidth;
    float columnMarginW         = 0.0333f;

    int colourButtonMargin;
    int colourButtonWidth;
    float colourButtonParentW   = 0.031f;

    int colourColumnHeight;
    float colourColumnH         = 0.6f;

    int colourPanelFix = 0;
};

#endif // LUMATONE_EDITOR_KEY_EDITOR_CONTROLS_H
