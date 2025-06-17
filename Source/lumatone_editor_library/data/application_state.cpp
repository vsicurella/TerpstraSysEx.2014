#include "application_state.h"

#include "../device/lumatone_controller.h"
#include "../device/activity_monitor.h"
#include "../device/lumatone_event_manager.h"

#include "../color/colour_model.h"
#include "../data/lumatone_context.h"
#include "../actions/lumatone_action.h"

#include "../listeners/status_listener.h"
#include "../listeners/editor_listener.h"
#include "../listeners/firmware_listener.h"
#include "../listeners/midi_listener.h"

#include "../lumatone_midi_driver/lumatone_midi_driver.h"
#include "../lumatone_midi_driver/firmware_types.h"

juce::Array<juce::Identifier> getLumatoneApplicationProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneApplicationProperty::ConnectionStateId);
    properties.add(LumatoneApplicationProperty::LayoutContextIsSetId);
    return properties;
}

LumatoneApplicationState::LumatoneApplicationState(juce::ValueTree stateIn, LumatoneFirmwareDriver& driverIn, juce::UndoManager *undoManagerIn)
    : LumatoneState(stateIn, undoManagerIn)
    , firmwareDriver(driverIn)
{
    editorListeners = std::make_shared<juce::ListenerList<LumatoneEditor::EditorListener>>();
    statusListeners = std::make_shared<juce::ListenerList<LumatoneEditor::StatusListener>>();
    firmwareListeners = std::make_shared<juce::ListenerList<LumatoneEditor::FirmwareListener>>();
    midiListeners = std::make_shared<juce::ListenerList<LumatoneEditor::MidiListener>>();

    selectedKeys = std::make_shared<juce::Array<MappedLumatoneKey>>();

    receiveSettingsStatus = std::make_shared<FirmwareSupport::ReceiveSettingsStatus>();
    receiveLayoutStatus = std::make_shared<FirmwareSupport::ReceiveLayoutStatus>();

    layoutContext = std::make_shared<LumatoneContext>(*mappingData);
    colourModel = std::make_shared<LumatoneColourModel>();

    eventManager = std::make_shared<LumatoneEventManager>(*this, driverIn);
	controller = std::make_shared<LumatoneController>(*this, driverIn);
    activityMonitor = std::make_shared<DeviceActivityMonitor>(*this, driverIn);

    loadStateProperties(stateIn);
}

LumatoneApplicationState::LumatoneApplicationState(juce::String nameIn, const LumatoneApplicationState &stateIn)
    : LumatoneState(nameIn, (const LumatoneState&)stateIn)
    , firmwareDriver(stateIn.firmwareDriver)
    , editorListeners(stateIn.editorListeners)
    , statusListeners(stateIn.statusListeners)
    , firmwareListeners(stateIn.firmwareListeners)
    , midiListeners(stateIn.midiListeners)
    , selectedKeys(stateIn.selectedKeys)
    , receiveSettingsStatus(stateIn.receiveSettingsStatus)
    , receiveLayoutStatus(stateIn.receiveLayoutStatus)
    , layoutContext(stateIn.layoutContext)
    , colourModel(stateIn.colourModel)
    , eventManager(stateIn.eventManager)
    , controller(stateIn.controller)
    , activityMonitor(stateIn.activityMonitor)
{
    loadStateProperties(state);
}

LumatoneApplicationState::LumatoneApplicationState(const LumatoneApplicationState &stateIn)
    : LumatoneApplicationState(stateIn.name + "Copy", stateIn)
{
}

LumatoneApplicationState::~LumatoneApplicationState()
{
    activityMonitor = nullptr;
    controller = nullptr;
    eventManager = nullptr;
    colourModel = nullptr;
    layoutContext = nullptr;
    receiveLayoutStatus = nullptr;
    receiveSettingsStatus = nullptr;
    midiListeners = nullptr;
    firmwareListeners = nullptr;
    statusListeners = nullptr;
    editorListeners = nullptr;
}

ConnectionState LumatoneApplicationState::getConnectionState() const
{
    return connectionState;
}

int LumatoneApplicationState::getMidiInputIndex() const
{
    return firmwareDriver.getMidiInputIndex();
}

int LumatoneApplicationState::getMidiOutputIndex() const
{
    return firmwareDriver.getMidiOutputIndex();
}

bool LumatoneApplicationState::isAutoConnectionEnabled() const
{
    return activityMonitor->willDetectDeviceIfDisconnected();
}

bool LumatoneApplicationState::doSendChangesToDevice() const
{
    return connectionState == ConnectionState::ONLINE;
}

const juce::Array<MappedLumatoneKey> *LumatoneApplicationState::getSelectedKeys() const
{
    return selectedKeys.get();
}

LumatoneController *LumatoneApplicationState::getLumatoneController() const
{
    return controller.get();
}

LumatoneColourModel *LumatoneApplicationState::getColourModel() const
{
    return colourModel.get();
}

const LumatoneContext *LumatoneApplicationState::getContext() const
{
    return contextIsSet ? layoutContext.get() : nullptr;
}

std::shared_ptr<LumatoneContext> LumatoneApplicationState::shareContext()
{
    return layoutContext;
}

bool LumatoneApplicationState::performLumatoneAction(LumatoneAction *action, bool undoable, bool newTransaction)
{
    if (action == nullptr)
        return false;

    if (undoable)
    {
        if (undoManager == nullptr)
            return false;

        if (newTransaction)
            undoManager->beginNewTransaction();

        return undoManager->perform((juce::UndoableAction*)action, action->getName());
    }

    return action->perform();
}

void LumatoneApplicationState::setInactiveMacroButtonColour(juce::Colour buttonColour)
{
    LumatoneState::setInactiveMacroButtonColour(buttonColour);

    if (doSendChangesToDevice())
    {
        controller->sendMacroButtonInactiveColour(buttonColour.toString());
    }

    editorListeners->call(&LumatoneEditor::EditorListener::macroButtonInactiveColourChanged, buttonColour);
}

void LumatoneApplicationState::setActiveMacroButtonColour(juce::Colour buttonColour)
{
    LumatoneState::setInactiveMacroButtonColour(buttonColour);

    if (doSendChangesToDevice())
    {
        controller->sendMacroButtonInactiveColour(buttonColour.toString());
    }

    editorListeners->call(&LumatoneEditor::EditorListener::macroButtonActiveColourChanged, buttonColour);
}

juce::ValueTree LumatoneApplicationState::loadStateProperties(juce::ValueTree stateIn)
{
    // juce::ValueTree newState = stateIn.isValid()
    //                          ? stateIn
    //                          : juce::ValueTree(LumatoneStateProperty::DefaultState);

    // LumatoneState::loadStateProperties(newState);

    // DBG("LumatoneApplicationState::loadStateProperties:\n" + newState.toXmlString());
    for (auto property : getLumatoneApplicationProperties())
    {
        if (stateIn.hasProperty(property))
            handleStatePropertyChange(stateIn, property);
    }

    // if (name.contains("Copy"))
    //     DBG(juce::String("Loaded ") + name + juce::String(" properties"));

    return stateIn;
}

void LumatoneApplicationState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == LumatoneApplicationProperty::ConnectionStateId)
    {
        connectionState = ConnectionState((int)stateIn.getProperty(property, (int)ConnectionState::DISCONNECTED));
    }
    else if (property == LumatoneApplicationProperty::LayoutContextIsSetId)
    {
        contextIsSet = (bool)stateIn.getProperty(property, false);
    }
    else
    {
        LumatoneState::handleStatePropertyChange(stateIn, property);
    }
}

void LumatoneApplicationState::loadPropertiesFile(juce::PropertiesFile *properties)
{
    LumatoneState::loadPropertiesFile(properties);

    setStateProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, properties->getBoolValue(LumatoneApplicationProperty::DetectDeviceIfDisconnected.toString(), true));
    setStateProperty(LumatoneApplicationProperty::CheckConnectionIfInactive, properties->getBoolValue(LumatoneApplicationProperty::CheckConnectionIfInactive.toString(), true));

    setStateProperty(LumatoneApplicationProperty::LastInputDeviceId, properties->getValue(LumatoneApplicationProperty::LastInputDeviceId.toString(), juce::String()));
    setStateProperty(LumatoneApplicationProperty::LastOutputDeviceId, properties->getValue(LumatoneApplicationProperty::LastOutputDeviceId.toString(), juce::String()));
}

LumatoneKeyContext LumatoneApplicationState::getKeyContext(int boardIndex, int keyIndex) const
{
    if (contextIsSet)
    {
        return layoutContext->getKeyContext(boardIndex, keyIndex);
    }

    MappedLumatoneKey key = MappedLumatoneKey(getKey(boardIndex, keyIndex), boardIndex, keyIndex);
    return LumatoneKeyContext(key);
}

void LumatoneApplicationState::setContext(const LumatoneContext& contextIn)
{
    if (layoutContext.get() != nullptr)
        clearContext();

    *layoutContext = contextIn;
    contextIsSet = true;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}

void LumatoneApplicationState::clearContext()
{
    *layoutContext = LumatoneContext(*mappingData);
    contextIsSet = false;

    state.setPropertyExcludingListener(this, LumatoneApplicationProperty::LayoutContextIsSetId, contextIsSet, undoManager);
}

void LumatoneApplicationState::setCompleteConfig(const LumatoneLayout &layoutIn)
{
    LumatoneState::setCompleteConfig(layoutIn);

    if (doSendChangesToDevice())
    {
        controller->sendCurrentCompleteConfig();
    }

    editorListeners->call(&LumatoneEditor::EditorListener::layoutChanged, *mappingData);
    editorListeners->call(&LumatoneEditor::EditorListener::expressionPedalSensitivityChanged, getExpressionSensitivity());
    editorListeners->call(&LumatoneEditor::EditorListener::invertFootControllerChanged, getInvertExpression());
    editorListeners->call(&LumatoneEditor::EditorListener::lightOnKeyStrokesChanged, getLightOnKeyStrokes());
    editorListeners->call(&LumatoneEditor::EditorListener::aftertouchToggled, getAftertouchOn());

    editorListeners->call(&LumatoneEditor::EditorListener::configTableChanged, LumatoneConfigTable::TableType::velocityInterval);
    editorListeners->call(&LumatoneEditor::EditorListener::configTableChanged, LumatoneConfigTable::TableType::fader);
    editorListeners->call(&LumatoneEditor::EditorListener::configTableChanged, LumatoneConfigTable::TableType::afterTouch);
    editorListeners->call(&LumatoneEditor::EditorListener::configTableChanged, LumatoneConfigTable::TableType::lumaTouch);
}

void LumatoneApplicationState::setLayout(const LumatoneLayout &layoutIn)
{
    LumatoneState::setLayout(layoutIn);

    if (doSendChangesToDevice())
    {
        controller->sendCompleteMapping(layoutIn, false, false);
    }

    clearContext();

    editorListeners->call(&LumatoneEditor::EditorListener::layoutChanged, *mappingData);
}

void LumatoneApplicationState::setBoard(const LumatoneBoard &boardIn, int boardId)
{
    LumatoneState::setBoard(boardIn, boardId);

    if (doSendChangesToDevice())
    {
        controller->sendAllParamsOfBoard(boardId, &boardIn);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::boardChanged, getBoard(boardId-1));
}

void LumatoneApplicationState::setKey(const LumatoneKey &keyIn, int boardId, int keyIndex)
{
    LumatoneState::setKey(keyIn, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyParam(boardId, keyIndex, keyIn);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}

void LumatoneApplicationState::setKeyConfig(const LumatoneKey& keyIn, int boardId, int keyIndex)
{
    LumatoneState::setKeyConfig(keyIn, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyConfig(boardId, keyIndex, keyIn);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}


void LumatoneApplicationState::setKeyColour(juce::Colour colour, int boardId, int keyIndex)
{
    LumatoneState::setKeyColour(colour, boardId, keyIndex);

    if (doSendChangesToDevice())
    {
        controller->sendKeyColourConfig(boardId, keyIndex, colour);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::keyChanged, boardId - 1, keyIndex, getKey(boardId - 1, keyIndex));
}

void LumatoneApplicationState::sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    LumatoneState::sendSelectionParam(selection);

    controller->sendSelectionParam(selection, false, bufferKeyUpdates);

    //if (signalEditorListeners)
    editorListeners->call(&LumatoneEditor::EditorListener::keySetChanged, selection);
}

void LumatoneApplicationState::sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    LumatoneState::sendSelectionColours(selection);

    controller->sendSelectionColours(selection, false, bufferKeyUpdates);

    //if (signalEditorListeners)
    editorListeners->call(&LumatoneEditor::EditorListener::keySetChanged, selection);
}

void LumatoneApplicationState::setAftertouchEnabled(bool enabled)
{
    LumatoneState::setAftertouchEnabled(enabled);

    if (doSendChangesToDevice())
    {
        controller->setAftertouchEnabled(enabled);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::aftertouchToggled, enabled);
}

void LumatoneApplicationState::setLightOnKeyStrokes(bool enabled)
{
    LumatoneState::setLightOnKeyStrokes(enabled);

    if (doSendChangesToDevice())
    {
        controller->sendLightOnKeyStrokes(enabled);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::lightOnKeyStrokesChanged, enabled);
}

void LumatoneApplicationState::setInvertExpression(bool invert)
{
    LumatoneState::setInvertExpression(invert);

    if (doSendChangesToDevice())
    {
        controller->sendInvertFootController(invert);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::invertFootControllerChanged, invert);
}

void LumatoneApplicationState::setInvertSustain(bool invert)
{
    LumatoneState::setInvertSustain(invert);

    if (doSendChangesToDevice())
    {
        controller->setInvertSustainPedal(invert);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::invertSustainToggled, invert);
}

void LumatoneApplicationState::setExpressionSensitivity(juce::uint8 sensitivity)
{
    LumatoneState::setExpressionSensitivity(sensitivity);

    if (doSendChangesToDevice())
    {
        controller->sendExpressionPedalSensivity(sensitivity);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::expressionPedalSensitivityChanged, sensitivity);
}

void LumatoneApplicationState::setConfigTable(LumatoneConfigTable::TableType type, const LumatoneConfigTable& table)
{
    LumatoneState::setConfigTable(type, table);

    if (doSendChangesToDevice())
    {
        controller->sendTableConfig(type, table.velocityValues);
    }

    editorListeners->call(&LumatoneEditor::EditorListener::configTableChanged, type);
}
//
//void LumatoneApplicationState::setVelocityIntervalTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setVelocityIntervalTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setVelocityIntervalTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setNoteVelocityTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setNoteVelocityTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setNoteVelocityTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setAftertouchTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setAftertouchTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setAftertouchTable(tableIn);
//    }
//}
//
//void LumatoneApplicationState::setLumatouchTable(const LumatoneConfigTable& tableIn)
//{
//    LumatoneState::setLumatouchTable(tableIn);
//
//    if (doSendChangesToDevice())
//    {
//        controller->setLumatouchTable(tableIn);
//    }
//}

void LumatoneApplicationState::Controller::updateSelectionState(LumatoneApplicationState& stateIn, const juce::Array<MappedLumatoneKey>& selection)
{
    auto selectionState = stateIn.state.getOrCreateChildWithName(LumatoneApplicationProperty::KeySelection, nullptr);
    selectionState.removeAllChildren(nullptr);

    for (const MappedLumatoneKey& key : selection)
    {
        juce::ValueTree selectedKey(LumatoneApplicationProperty::SelectedKey);
        selectedKey.setProperty(LumatoneKeyProperty::Index, key.keyIndex, nullptr);
        selectedKey.setProperty(LumatoneKeyProperty::Board, key.boardIndex, nullptr);
        selectionState.addChild(selectedKey, -1, nullptr);
    }

    selectionState.setPropertyExcludingListener(&stateIn, LumatoneApplicationProperty::NumKeySelected, selection.size(), nullptr);


    // DBG(stateIn.state.toXmlString());
}

void LumatoneApplicationState::Controller::setSelectedKeys(juce::Array<MappedLumatoneKey> selection)
{
    appState.selectedKeys->swapWith(selection);

    updateSelectionState(appState, *appState.selectedKeys);

    getEditorListeners()->call(&LumatoneEditor::EditorListener::selectionChanged);
}

void LumatoneApplicationState::Controller::addKeyToSelection(int keyNum)
{
    LumatoneKeyCoord coords = appState.mappingData->keyNumToKeyCoord(keyNum);
    const MappedLumatoneKey key = appState.mappingData->getMappedKey(coords.boardIndex, coords.keyIndex);

    bool inserted = false;
    if (appState.selectedKeys->size() == 0)
    {
        appState.selectedKeys->add(key);
        inserted = true;
    }
    else for (int i = 0; i < appState.selectedKeys->size(); i++)
    {
        const MappedLumatoneKey& selectedKey = appState.selectedKeys->getReference(i);
        const LumatoneKeyCoord keyCoord = selectedKey.getKeyCoord();
        if (coords.boardIndex == keyCoord.boardIndex && coords.keyIndex == keyCoord.keyIndex)
        {
            appState.selectedKeys->remove(i);
            appState.selectedKeys->insert(i, key);
            inserted = true;
            break;
        }
        else if (selectedKey > key)
        {
            appState.selectedKeys->insert(jmax(0, i - 1), key);
            inserted = true;
            break;
        }
    }

    if (!inserted)
    {
        appState.selectedKeys->add(key);
    }

    updateSelectionState(appState, *appState.selectedKeys);
    getEditorListeners()->call(&LumatoneEditor::EditorListener::selectionChanged);
}

void LumatoneApplicationState::Controller::removeKeyFromSelection(int keyNum)
{
    LumatoneKeyCoord coords = appState.mappingData->keyNumToKeyCoord(keyNum);
    const MappedLumatoneKey key = appState.mappingData->getMappedKey(coords.boardIndex, coords.keyIndex);
    bool removed = false;
    for (int i = 0; i < appState.selectedKeys->size(); i++)
    {
        const LumatoneKeyCoord keyCoord = appState.selectedKeys->getReference(i).getKeyCoord();
        if (coords.boardIndex == keyCoord.boardIndex && coords.keyIndex == keyCoord.keyIndex)
        {
            appState.selectedKeys->remove(i);
            removed = true;
            break;
        }

    }

    if (removed)
    {
        updateSelectionState(appState, *appState.selectedKeys);
        getEditorListeners()->call(&LumatoneEditor::EditorListener::selectionChanged);
    }
}

void LumatoneApplicationState::Controller::clearSelectedKeys()
{
    juce::Array<MappedLumatoneKey> emptySelection;
    appState.selectedKeys->swapWith(emptySelection);
    updateSelectionState(appState, *appState.selectedKeys);
    getEditorListeners()->call(&LumatoneEditor::EditorListener::selectionChanged);
}

void LumatoneApplicationState::Controller::setColourMode(ColourModes mode)
{
    switch (mode)
    {
    case ColourModes::None:
        appState.setStateProperty(LumatoneApplicationProperty::ColourMode, "");
        break;
    case ColourModes::RGB:
        appState.setStateProperty(LumatoneApplicationProperty::ColourMode, "RGB");
        break;

    default:
    case ColourModes::ModelAdjusted:
        appState.setStateProperty(LumatoneApplicationProperty::ColourMode, "ModelAdjusted");
        appState.colourModel->setType(LumatoneColourModel::Type::ADJUSTED);
        break;

    // TODO raw/other adjustments
    }
}

void LumatoneApplicationState::Controller::updatedSelectedKeys()
{
    juce::Array<MappedLumatoneKey> updatedKeys;
    for (const MappedLumatoneKey& key : *appState.selectedKeys)
    {
        updatedKeys.add(MappedLumatoneKey(appState.getKey(key.boardIndex, key.keyIndex), key.boardIndex, key.keyIndex));
    }

    appState.selectedKeys->swapWith(updatedKeys);
    appState.editorListeners->call(&LumatoneEditor::EditorListener::selectionChanged);
}

FirmwareSupport::ReceiveSettingsStatus &LumatoneApplicationState::Controller::getReceivedSettingsStatus()
{
    return *appState.receiveSettingsStatus;
}

FirmwareSupport::ReceiveLayoutStatus &LumatoneApplicationState::Controller::getReceivedLayoutStatus()
{
    return *appState.receiveLayoutStatus;
}

bool LumatoneApplicationState::Controller::performAction(LumatoneAction *action, bool undoable, bool newTransaction)
{
    return appState.performLumatoneAction(action, undoable, newTransaction);
}

void LumatoneApplicationState::addStatusListener(LumatoneEditor::StatusListener* listenerIn)
{
    statusListeners->add(listenerIn);
}

void LumatoneApplicationState::removeStatusListener(LumatoneEditor::StatusListener* listenerIn)
{
    statusListeners->remove(listenerIn);
}

void LumatoneApplicationState::addEditorListener(LumatoneEditor::EditorListener* listenerIn)
{
    editorListeners->add(listenerIn);
}

void LumatoneApplicationState::removeEditorListener(LumatoneEditor::EditorListener* listenerIn)
{
    editorListeners->remove(listenerIn);
}

void LumatoneApplicationState::addFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn)
{
    firmwareListeners->add(listenerIn);
}

void LumatoneApplicationState::removeFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn)
{
    firmwareListeners->remove(listenerIn);
}

void LumatoneApplicationState::addMidiListener(LumatoneEditor::MidiListener* listenerIn)
{
    midiListeners->add(listenerIn);
}

void LumatoneApplicationState::removeMidiListener(LumatoneEditor::MidiListener* listenerIn)
{
    midiListeners->remove(listenerIn);
}

bool LumatoneApplicationState::Controller::requestCompleteDeviceConfig()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    // requestDeviceGlobalSettings();
    requestDeviceMapping();

    return true;
}

bool LumatoneApplicationState::Controller::requestDeviceGlobalSettings()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    // Reset state for tracking response progress
    *appState.receiveSettingsStatus = FirmwareSupport::ReceiveSettingsStatus(appState.getLumatoneVersion());

    // Macro button colours
    appState.controller->requestMacroButtonColours();

	// General options
    appState.controller->getPeripheralChannels();

	// Velocity curve config
	appState.controller->sendVelocityIntervalConfigRequest();

    return true;
}

bool LumatoneApplicationState::Controller::requestDeviceMapping()
{
    if (appState.connectionState != ConnectionState::ONLINE)
        return false;

    // Reset state for tracking response progress
    *appState.receiveLayoutStatus = FirmwareSupport::ReceiveLayoutStatus(appState.getLumatoneVersion(), appState.getNumBoards());

    // Request mapping look-up tables
    appState.controller->sendVelocityConfigRequest();
	appState.controller->sendFaderConfigRequest();
	appState.controller->sendAftertouchConfigRequest();

    // Request settings associated with mapping
    appState.controller->requestExpressionPedalSensitivity();
    appState.controller->requestPresetFlags();

    // Request MIDI channel, MIDI note, colour and key type config for all keys
    // This is last as to be determinate if other settings/tables are added
    appState.controller->sendGetAllBoardsMappingRequest();

    return true;
}

void LumatoneApplicationState::Controller::setInactiveMacroButtonColour(juce::Colour buttonColour)
{
    appState.setInactiveMacroButtonColour(buttonColour);
}

void LumatoneApplicationState::Controller::setActiveMacroButtonColour(juce::Colour buttonColour)
{
    appState.setActiveMacroButtonColour(buttonColour);
}

void LumatoneApplicationState::DeviceController::setConnectionState(ConnectionState newState, bool sendNotification)
{
    bool stateChanged = deviceAppState.connectionState != newState;

    deviceAppState.connectionState = newState;
    deviceAppState.setStateProperty(LumatoneApplicationProperty::ConnectionStateId, juce::var((int)deviceAppState.connectionState));

    if (sendNotification)
        getStatusListeners()->call(&LumatoneEditor::StatusListener::connectionStateChanged, deviceAppState.connectionState);
}

void LumatoneApplicationState::DeviceController::setAutoConnectionEnabled(bool enabled)
{
    deviceAppState.activityMonitor->setDetectDeviceIfDisconnected(enabled);
    deviceAppState.activityMonitor->setCheckForInactivity(enabled);
}

juce::Array<juce::MidiDeviceInfo> LumatoneApplicationState::DeviceController::getMidiInputList()
{
    return deviceAppState.firmwareDriver.getMidiInputList();
}

juce::Array<juce::MidiDeviceInfo> LumatoneApplicationState::DeviceController::getMidiOutputList()
{
    return deviceAppState.firmwareDriver.getMidiOutputList();
}

void LumatoneApplicationState::DeviceController::setMidiInput(int deviceIndex, bool test)
{
    auto deviceInfo = getMidiInputList()[deviceIndex];
    deviceAppState.setStateProperty(LumatoneApplicationProperty::LastInputDeviceId, deviceInfo.identifier);
    deviceAppState.activityMonitor->setMidiInput(deviceIndex, test);
}

void LumatoneApplicationState::DeviceController::setMidiOutput(int deviceIndex, bool test)
{
    auto deviceInfo = getMidiOutputList()[deviceIndex];
    deviceAppState.setStateProperty(LumatoneApplicationProperty::LastOutputDeviceId, deviceInfo.identifier);
    deviceAppState.activityMonitor->setMidiOutput(deviceIndex, test);
}
