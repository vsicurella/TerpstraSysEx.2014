#include "../lumatone_editor_library/ui/keyboard_component.h"
#include "../data/LumatoneEditorState.h"
#include "../lumatone_editor_library/palettes/colour_selection_broadcaster.h"

class LumatoneEditorKeyboardComponent : public LumatoneKeyboardComponent
                                      , private LumatoneEditorState
                                      , private LumatoneEditorState::Controller
                                      , public ColourSelectionBroadcaster

{
public:

    LumatoneEditorKeyboardComponent(const LumatoneEditorState& stateIn);
    ~LumatoneEditorKeyboardComponent() override;

protected:

    // juce::Component UI implementations
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDragInternalOnNewKey(const juce::MouseEvent& e, LumatoneKeyDisplay* key) override;

private:
    bool keyStateChanged(bool isKeyDown) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    void modifierKeysChanged(const juce::ModifierKeys& modifiers) override;

private:

    void noteOnInternal(int midiChannel, int midiNote, juce::uint8 velocity) override;
    // void noteOffInternal(int midiChannel, int midiNote) override;

protected:

    juce::Colour getSelectedColour() override;
    void deselectColour() override;

protected:
    bool shiftHeld = false;
    bool altHeld = false;
    bool ctrlHeld = false;

    bool mouseWasDragging = false;

    juce::Colour selectedColour;
};
