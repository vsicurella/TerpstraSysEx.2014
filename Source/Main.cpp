/*
  ==============================================================================

	This file was auto-generated by the Introjucer!

	It contains the basic startup code for a Juce application.
	Created: xx.xx.2014

  ==============================================================================
*/

#include "Main.h"
#include "GeneralOptionsDlg.h"
#include "VelocityCurveDlgBase.h"
#include "NoteOnOffVelocityCurveDialog.h"
#include "LumatoneMenu.h"

//==============================================================================

MainContentComponent* TerpstraSysExApplication::getMainContentComponent()
{
	jassert(mainWindow != nullptr);
	return (MainContentComponent*)(mainWindow->getContentComponent());
}

TerpstraSysExApplication::TerpstraSysExApplication()
	: lookAndFeel(appFonts.fonts, true), tooltipWindow(), hasChangesToSave(false)
{
	PropertiesFile::Options options;
	options.applicationName = "LumatoneSetup";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
	options.folderName = "~/.config/LumatoneSetup";
#else
	options.folderName = "LumatoneSetup";
#endif
	propertiesFile = new PropertiesFile(options);
	jassert(propertiesFile != nullptr);

	octaveBoardSize = propertiesFile->getBoolValue("55Keys", false) ? 55 : 56;

	int manufacturerId = propertiesFile->getIntValue("ManufacturerId", 0x002150);
	midiDriver.setManufacturerId(manufacturerId);

	deviceMonitor.reset(new DeviceActivityMonitor(midiDriver, propertiesFile->getIntValue("DeviceDetectTimeout", 700)));

	// Localisation
	String localisation = getLocalisation(SystemStats::getDisplayLanguage());
	LocalisedStrings::setCurrentMappings(new LocalisedStrings(localisation, false));
	LocalisedStrings::getCurrentMappings()->setFallback(new LocalisedStrings(BinaryData::engb_txt, false));

	// Window aspect ratio
	boundsConstrainer.reset(new ComponentBoundsConstrainer());
	boundsConstrainer->setFixedAspectRatio(DEFAULTMAINWINDOWASPECT);
	boundsConstrainer->setMinimumSize(800, round(800 / DEFAULTMAINWINDOWASPECT));

	lookAndFeel.setColour(juce::ComboBox::arrowColourId, Colour(0xfff7990d));
	lookAndFeel.setColour(juce::ToggleButton::tickColourId, Colour(0xfff7990d));

	lookAndFeel.setColour(TerpstraKeyEdit::backgroundColourId, lookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));
	lookAndFeel.setColour(TerpstraKeyEdit::outlineColourId, Colour(0xffd7d9da));
	lookAndFeel.setColour(TerpstraKeyEdit::selectedKeyOutlineId, Colour(0xfff7990d));

	lookAndFeel.setColour(VelocityCurveBeam::beamColourId, Colour(0x66ff5e00));
	lookAndFeel.setColour(VelocityCurveBeam::outlineColourId, Colour(0xffd7d9da));

	// Recent files list
	recentFiles.restoreFromString(propertiesFile->getValue("RecentFiles"));
	recentFiles.removeNonExistentFiles();

	// Save/Load location preferences or default fallback values

	String possibleDirectory = propertiesFile->getValue("UserDocumentsDirectory");
	if (File::isAbsolutePath(possibleDirectory))
	{
		userDocumentsDirectory = File(possibleDirectory);
	}
	if (!userDocumentsDirectory.exists() || userDocumentsDirectory.existsAsFile())
	{
		userDocumentsDirectory = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Lumatone Editor");
		userDocumentsDirectory.createDirectory();
	}

	possibleDirectory = propertiesFile->getValue("UserMappingsDirectory");
	if (File::isAbsolutePath(possibleDirectory))
	{
		userMappingsDirectory = File(possibleDirectory);
	}
	if (!userMappingsDirectory.exists() || userMappingsDirectory.existsAsFile())
	{
		userMappingsDirectory = userDocumentsDirectory.getChildFile("Mappings");
		userMappingsDirectory.createDirectory();
	}

	possibleDirectory = propertiesFile->getValue("UserPalettesDirectory");
	if (File::isAbsolutePath(possibleDirectory))
	{
		userPalettesDirectory = File(possibleDirectory);
	}
	if (!userPalettesDirectory.exists() || userPalettesDirectory.existsAsFile())
	{
		userPalettesDirectory = userDocumentsDirectory.getChildFile("Palettes");
		userPalettesDirectory.createDirectory();
	}

	reloadColourPalettes();

	// State of main window will be read from properties file when main window is created
}

//==============================================================================
void TerpstraSysExApplication::initialise(const String& commandLine)
{
	// This method is where you should put your application's initialisation code..

	// commandLine parameters
	if (!commandLine.isEmpty())
	{
		auto commandLineParameters = getCommandLineParameterArray();

		for (auto commandLineParameter : commandLineParameters)
		{
			auto paramInLowerCase = commandLineParameter.toLowerCase();
			if (paramInLowerCase == "-55keys" || paramInLowerCase == "/55keys")
			{
				octaveBoardSize = 55;
				jassert(propertiesFile != nullptr);
				propertiesFile->setValue("55Keys", true);
				continue;
			}

			if (paramInLowerCase == "-56keys" || paramInLowerCase == "/56keys")
			{
				octaveBoardSize = 56;
				jassert(propertiesFile != nullptr);
				propertiesFile->setValue("55Keys", false);
				continue;
			}

			// ToDo switch on/off isomorphic mass assign mode

			// Try to open a config file
			if (File::isAbsolutePath(commandLineParameter))
			{
				currentFile = File(commandLineParameter);
			}
			else
			{
				// If file name is with quotes, try removing the quotes
				if (commandLine.startsWithChar('"') && commandLine.endsWithChar('"'))
					currentFile = File(commandLine.substring(1, commandLine.length() - 1));
			}

			if (currentFile.existsAsFile())
				break;	// There can only be one file, and the file name is supposed to be the last parameter
		}
	}

	commandManager.reset(new ApplicationCommandManager());
	commandManager->registerAllCommandsForTarget(this);

	menuModel.reset(new Lumatone::Menu::MainMenuModel(commandManager.get()));

	mainWindow.reset(new MainWindow());
	mainWindow->setMenuBar(menuModel.get());
	mainWindow->addKeyListener(commandManager->getKeyMappings());
	mainWindow->restoreStateFromPropertiesFile(propertiesFile);

	if (currentFile.existsAsFile())
		openFromCurrentFile();
}

void TerpstraSysExApplication::shutdown()
{
	// Add your application's shutdown code here..

	// Save documents directories (Future: provide option to change them and save after changed by user)
	propertiesFile->setValue("UserDocumentsDirectory", userDocumentsDirectory.getFullPathName());
	propertiesFile->setValue("UserMappingsDirectory", userMappingsDirectory.getFullPathName());
	propertiesFile->setValue("UserPalettesDirectory", userPalettesDirectory.getFullPathName());

	// Save recent files list
	recentFiles.removeNonExistentFiles();
	jassert(propertiesFile != nullptr);
	propertiesFile->setValue("RecentFiles", recentFiles.toString());

	// Save state of main window
	mainWindow->saveStateToPropertiesFile(propertiesFile);

	propertiesFile->saveIfNeeded();
	delete propertiesFile;
	propertiesFile = nullptr;

	LocalisedStrings::setCurrentMappings(nullptr);

	mainWindow = nullptr; // (deletes our window)
	//commandManager = nullptr;
	deviceMonitor = nullptr;
}

//==============================================================================
void TerpstraSysExApplication::systemRequestedQuit()
{
	// This is called when the app is being asked to quit: you can ignore this
	// request and let the app carry on running, or call quit() to allow the app to close.

	// If there are changes: ask for save
	if (hasChangesToSave)
	{
		int retc = AlertWindow::showYesNoCancelBox(AlertWindow::AlertIconType::QuestionIcon, "Quitting the application", "Do you want to save your changes?");
		if (retc == 0)
		{
			// "Cancel". Do not quit.
			return;
		}
		else if (retc == 1)
		{
			// "Yes". Try to save. Cancel if unsuccessful
			if (!saveSysExMapping())
				return;
		}
		// retc == 2: "No" -> end without saving
	}

	quit();
}

void TerpstraSysExApplication::anotherInstanceStarted(const String& commandLine)
{
	// When another instance of the app is launched while this one is running,
	// this method is invoked, and the commandLine parameter tells you what
	// the other instance's command-line arguments were.
}

void TerpstraSysExApplication::reloadColourPalettes()
{
	auto foundPaletteFiles = userPalettesDirectory.findChildFiles(File::TypesOfFileToFind::findFiles, true, '*' + String(PALETTEFILEEXTENSION));

	colourPalettes.clear();

	auto paletteSorter = LumatoneEditorPaletteSorter();
	for (auto file : foundPaletteFiles)
	{
		LumatoneEditorColourPalette palette = LumatoneEditorColourPalette::loadFromFile(file);
		colourPalettes.addSorted(paletteSorter, palette);
	}

}

bool TerpstraSysExApplication::saveColourPalette(LumatoneEditorColourPalette& palette, File pathToFile)
{
	bool success = false;

	if (palette.hasBeenModified())
	{
		ValueTree paletteNode = palette.toValueTree();

		// New file
		if (!pathToFile.existsAsFile())
		{
			if (palette.getName() != String())
				pathToFile = userPalettesDirectory.getChildFile(palette.getName());
			else
				pathToFile = userPalettesDirectory.getChildFile("UnnamedPalette");

			// Make sure filename is unique since saving happens automatically
			int nameId = 0;

			// One thousand should be enough...right?
			while (pathToFile.withFileExtension(PALETTEFILEEXTENSION).existsAsFile() && nameId < 1000)
			{
				nameId++;
				pathToFile = userPalettesDirectory.getChildFile("UnnamedPalette" + String(nameId));
			}
		}

		success = palette.saveToFile(pathToFile);

		// TODO error handling?
	}

	if (success)
		reloadColourPalettes();

	return success;
}

bool TerpstraSysExApplication::deletePaletteFile(File pathToPalette)
{
	bool success = false;

	if (pathToPalette.existsAsFile())
	{
		success = pathToPalette.deleteFile();
	}

	return success;
}

void TerpstraSysExApplication::getAllCommands(Array <CommandID>& commands)
{
	JUCEApplication::getAllCommands(commands);

	const CommandID ids[] = {
		Lumatone::Menu::commandIDs::openSysExMapping,
		Lumatone::Menu::commandIDs::saveSysExMapping,
		Lumatone::Menu::commandIDs::saveSysExMappingAs,
		Lumatone::Menu::commandIDs::resetSysExMapping,

		Lumatone::Menu::commandIDs::deleteOctaveBoard,
		Lumatone::Menu::commandIDs::copyOctaveBoard,
		Lumatone::Menu::commandIDs::pasteOctaveBoard,
		Lumatone::Menu::commandIDs::undo,
		Lumatone::Menu::commandIDs::redo,

		Lumatone::Debug::commandIDs::toggleDeveloperMode,

		Lumatone::Menu::commandIDs::aboutSysEx
	};

	commands.addArray(ids, numElementsInArray(ids));
}

void TerpstraSysExApplication::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case Lumatone::Menu::commandIDs::openSysExMapping:
		result.setInfo("Load file mapping", "Open a Lumatone key mapping", "File", 0);
		result.addDefaultKeypress('o', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::saveSysExMapping:
		result.setInfo("Save mapping", "Save the current mapping to file", "File", 0);
		result.addDefaultKeypress('s', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::saveSysExMappingAs:
		result.setInfo("Save mapping as...", "Save the current mapping to new file", "File", 0);
		result.addDefaultKeypress('a', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::resetSysExMapping:
		result.setInfo("New", "Start new mapping. Clear all edit fields, do not save current edits.", "File", 0);
		result.addDefaultKeypress('n', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::deleteOctaveBoard:
		result.setInfo("Delete", "Delete section data", "Edit", 0);
		result.addDefaultKeypress(KeyPress::deleteKey, ModifierKeys::noModifiers);
		break;

	case Lumatone::Menu::commandIDs::copyOctaveBoard:
		result.setInfo("Copy", "Copy section data", "Edit", 0);
		result.addDefaultKeypress('c', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::pasteOctaveBoard:
		result.setInfo("Paste", "Paste section data", "Edit", 0);
		result.addDefaultKeypress('v', ModifierKeys::ctrlModifier);
		break;

	case Lumatone::Menu::commandIDs::undo:
		result.setInfo("Undo", "Undo latest edit", "Edit", 0);
		result.addDefaultKeypress('z', ModifierKeys::ctrlModifier);
		result.setActive(undoManager.canUndo());
		break;

	case Lumatone::Menu::commandIDs::redo:
		result.setInfo("Redo", "Redo latest edit", "Edit", 0);
		result.addDefaultKeypress('y', ModifierKeys::ctrlModifier);
		result.setActive(undoManager.canRedo());
		break;

	case Lumatone::Menu::commandIDs::aboutSysEx:
		result.setInfo("About Lumatone Editor", "Shows version and copyright", "Help", 0);
		break;

	case Lumatone::Debug::commandIDs::toggleDeveloperMode:
		result.setInfo("Toggle Developer Mode", "Show/hide controls for tweaking internal parameters", "Edit", 0);
		result.addDefaultKeypress('m', juce::ModifierKeys::allKeyboardModifiers);
		break;

	default:
		JUCEApplication::getCommandInfo(commandID, result);
		break;
	}
}

bool TerpstraSysExApplication::perform(const InvocationInfo& info)
{
	switch (info.commandID)
	{
	case Lumatone::Menu::commandIDs::openSysExMapping:
		return openSysExMapping();
	case Lumatone::Menu::commandIDs::saveSysExMapping:
		return saveSysExMapping();
	case Lumatone::Menu::commandIDs::saveSysExMappingAs:
		return saveSysExMappingAs();
	case Lumatone::Menu::commandIDs::resetSysExMapping:
		return resetSysExMapping();
	case Lumatone::Menu::commandIDs::deleteOctaveBoard:

		return deleteSubBoardData();
	case Lumatone::Menu::commandIDs::copyOctaveBoard:
		return copySubBoardData();
	case Lumatone::Menu::commandIDs::pasteOctaveBoard:
		return pasteSubBoardData();

	case Lumatone::Menu::commandIDs::undo:
		return undo();

	case Lumatone::Menu::commandIDs::redo:
		return redo();

	case Lumatone::Menu::commandIDs::aboutSysEx:
		return aboutTerpstraSysEx();

	case Lumatone::Debug::commandIDs::toggleDeveloperMode:
		return toggleDeveloperMode();
	default:
		return JUCEApplication::perform(info);
	}
}

bool TerpstraSysExApplication::openSysExMapping()
{
	FileChooser chooser("Open a Lumatone key mapping", recentFiles.getFile(0).getParentDirectory(), "*.ltn;*.tsx");
	if (chooser.browseForFileToOpen())
	{
		currentFile = chooser.getResult();
		return openFromCurrentFile();
	}
	return true;
}

bool TerpstraSysExApplication::saveSysExMapping()
{
	if (currentFile.getFileName().isEmpty())
		return saveSysExMappingAs();
	else
		return saveCurrentFile();

}

bool TerpstraSysExApplication::saveSysExMappingAs()
{
	FileChooser chooser("Lumatone Key Mapping Files", recentFiles.getFile(0).getParentDirectory(), "*.ltn");
	if (chooser.browseForFileToSave(true))
	{
		currentFile = chooser.getResult();
		if (saveCurrentFile())
		{
			// Window title
			updateMainTitle();
			return true;
		}
	}

	return false;
}

bool TerpstraSysExApplication::resetSysExMapping()
{
	// Clear file
	currentFile = File();

	// Clear all edit fields
	((MainContentComponent*)(mainWindow->getContentComponent()))->deleteAll();

	setHasChangesToSave(false);

	// Clear undoable actions
	// ToDo (?)
	undoManager.clearUndoHistory();


	// Window title
	updateMainTitle();

	return true;
}

bool TerpstraSysExApplication::deleteSubBoardData()
{
	return performUndoableAction(((MainContentComponent*)(mainWindow->getContentComponent()))->createDeleteCurrentSectionAction());
}

bool TerpstraSysExApplication::copySubBoardData()
{
	return ((MainContentComponent*)(mainWindow->getContentComponent()))->copyCurrentSubBoardData();
}

bool TerpstraSysExApplication::pasteSubBoardData()
{
	return performUndoableAction(((MainContentComponent*)(mainWindow->getContentComponent()))->createPasteCurrentSectionAction());;
}

bool TerpstraSysExApplication::performUndoableAction(UndoableAction* editAction)
{
	if (editAction != nullptr)
	{
		undoManager.beginNewTransaction();
		if (undoManager.perform(editAction))	// UndoManager will check for nullptr and also for disposing of the object
		{
			setHasChangesToSave(true);
			((MainContentComponent*)(mainWindow->getContentComponent()))->refreshKeyDataFields();
			return true;
		}
	}

	return false;
}

bool TerpstraSysExApplication::undo()
{
	if (undoManager.undo())
	{
		setHasChangesToSave(true);
		((MainContentComponent*)(mainWindow->getContentComponent()))->refreshKeyDataFields();
		return true;
	}
	else
		return false;
}

bool TerpstraSysExApplication::redo()
{
	if (undoManager.redo())
	{
		setHasChangesToSave(true);
		((MainContentComponent*)(mainWindow->getContentComponent()))->refreshKeyDataFields();
		return true;
	}
	else
		return false;
}

bool TerpstraSysExApplication::toggleDeveloperMode()
{
	bool newMode = !propertiesFile->getBoolValue("DeveloperMode");
	propertiesFile->setValue("DeveloperMode", newMode);
	return ((MainContentComponent*)(mainWindow->getContentComponent()))->setDeveloperMode(newMode);
}

bool TerpstraSysExApplication::generalOptionsDialog()
{
	GeneralOptionsDlg* optionsWindow = new GeneralOptionsDlg();
	optionsWindow->setLookAndFeel(&lookAndFeel);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(optionsWindow);
	launchOptions.content->setSize(480, 240);

	launchOptions.dialogTitle = "General options";
	launchOptions.dialogBackgroundColour = lookAndFeel.findColour(juce::ResizableWindow::backgroundColourId);
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.useNativeTitleBar = false;
	launchOptions.resizable = true;

	DialogWindow* dw = launchOptions.launchAsync();
	dw->centreWithSize(480, 240);

	return true;
}

bool TerpstraSysExApplication::noteOnOffVelocityCurveDialog()
{
	NoteOnOffVelocityCurveDialog* velocityCurveWindow = new NoteOnOffVelocityCurveDialog();
	velocityCurveWindow->setLookAndFeel(&lookAndFeel);

	int dlgWidth = propertiesFile->getIntValue("VelocityCurveWindowWidth", 648);
	int dlgHeight = propertiesFile->getIntValue("VelocityCurveWindowHeight", 424);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(velocityCurveWindow);
	launchOptions.content->setSize(dlgWidth, dlgHeight);

	launchOptions.dialogTitle = "Note on/off velocity curve";
	launchOptions.dialogBackgroundColour = lookAndFeel.findColour(ResizableWindow::backgroundColourId);
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.useNativeTitleBar = false;
	launchOptions.resizable = true;

	DialogWindow* dw = launchOptions.launchAsync();
	dw->centreWithSize(dlgWidth, dlgHeight);

	return true;
}

bool TerpstraSysExApplication::faderVelocityCurveDialog()
{
	VelocityCurveDlgBase* velocityCurveWindow = new VelocityCurveDlgBase(TerpstraVelocityCurveConfig::VelocityCurveType::fader);
	velocityCurveWindow->setLookAndFeel(&lookAndFeel);

	int dlgWidth = propertiesFile->getIntValue("FaderVelocityCurveWindowWidth", 648);
	int dlgHeight = propertiesFile->getIntValue("FaderVelocityCurveWindowHeight", 424);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(velocityCurveWindow);
	launchOptions.content->setSize(dlgWidth, dlgHeight);

	launchOptions.dialogTitle = "Fader velocity curve";
	launchOptions.dialogBackgroundColour = lookAndFeel.findColour(ResizableWindow::backgroundColourId);
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.useNativeTitleBar = false;
	launchOptions.resizable = true;

	DialogWindow* dw = launchOptions.launchAsync();
	dw->centreWithSize(dlgWidth, dlgHeight);

	return true;
}

bool TerpstraSysExApplication::aftertouchVelocityCurveDialog()
{
	VelocityCurveDlgBase* velocityCurveWindow = new VelocityCurveDlgBase(TerpstraVelocityCurveConfig::VelocityCurveType::afterTouch);
	velocityCurveWindow->setLookAndFeel(&lookAndFeel);

	int dlgWidth = propertiesFile->getIntValue("AftertouchVelocityCurveWindowWidth", 768);
	int dlgHeight = propertiesFile->getIntValue("AftertouchVelocityCurveWindowHeight", 424);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(velocityCurveWindow);
	launchOptions.content->setSize(dlgWidth, dlgHeight);

	launchOptions.dialogTitle = "Aftertouch parameters";
	launchOptions.dialogBackgroundColour = lookAndFeel.findColour(ResizableWindow::backgroundColourId);
	launchOptions.escapeKeyTriggersCloseButton = true;
	launchOptions.useNativeTitleBar = false;
	launchOptions.resizable = true;

	DialogWindow* dw = launchOptions.launchAsync();
	dw->centreWithSize(dlgWidth, dlgHeight);

	return true;
}

// open a file from the "recent files" menu
bool TerpstraSysExApplication::openRecentFile(int recentFileIndex)
{
	jassert(recentFileIndex >= 0 && recentFileIndex < recentFiles.getNumFiles());
	currentFile = recentFiles.getFile(recentFileIndex);
	return openFromCurrentFile();
}

// Open a SysEx mapping from the file specified in currentFile
bool TerpstraSysExApplication::openFromCurrentFile()
{
	if (currentFile.existsAsFile())
	{
		// XXX StringArray format: platform-independent?
		StringArray stringArray;
		currentFile.readLines(stringArray);
		TerpstraKeyMapping keyMapping;
		keyMapping.fromStringArray(stringArray);

		((MainContentComponent*)(mainWindow->getContentComponent()))->setData(keyMapping);

		// Window title
		updateMainTitle();

		// Send configuration to controller, if connected
		sendCurrentConfigurationToDevice();

		// Mark file as unchanged
		setHasChangesToSave(false);

		// Clear undo history
		undoManager.clearUndoHistory();

		// Add file to recent files list
		recentFiles.addFile(currentFile);

		return true;
	}
	else
	{
		// Show error message
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Open File Error", "The file " + currentFile.getFullPathName() + " could not be opened.");

		// XXX Update Window title in any case? Make file name empty/make data empty in case of error?
		return false;
	}
}

// Saves the current mapping to file, specified in currentFile.
bool TerpstraSysExApplication::saveCurrentFile()
{
	if (currentFile.existsAsFile())
		currentFile.deleteFile();
	bool retc = currentFile.create();
	// XXX error handling

	TerpstraKeyMapping keyMapping;
	((MainContentComponent*)(mainWindow->getContentComponent()))->getData(keyMapping);

	StringArray stringArray = keyMapping.toStringArray();
	for (int i = 0; i < stringArray.size(); i++)
		currentFile.appendText(stringArray[i] + "\n");

	setHasChangesToSave(false);

	// ToDo undo history?

	// Add file to recent files list - or put it on top of the list
	recentFiles.addFile(currentFile);

	return retc;
}

void TerpstraSysExApplication::sendCurrentConfigurationToDevice()
{
	auto theConfig = ((MainContentComponent*)(mainWindow->getContentComponent()))->getMappingInEdit();

	// MIDI channel, MIDI note, colour and key type config for all keys
	getMidiDriver().sendCompleteMapping(theConfig);

	// General options
	getMidiDriver().sendAfterTouchActivation(theConfig.afterTouchActive);
	getMidiDriver().sendLightOnKeyStrokes(theConfig.lightOnKeyStrokes);
	getMidiDriver().sendInvertFootController(theConfig.invertFootController);
	getMidiDriver().sendExpressionPedalSensivity(theConfig.expressionControllerSensivity);

	// Velocity curve config
	getMidiDriver().sendVelocityIntervalConfig(theConfig.velocityIntervalTableValues);

	((MainContentComponent*)(mainWindow->getContentComponent()))->getCurvesArea()->sendConfigToController();
}

void TerpstraSysExApplication::requestConfigurationFromDevice()
{
	// if editing operations were done that have not been saved, give the possibility to save them
	if (hasChangesToSave)
	{
		int retc = AlertWindow::showYesNoCancelBox(
			AlertWindow::AlertIconType::QuestionIcon,
			"Request configuration from device",
			"The controller's current configuration will be received now. This will overwrite all edits you have done. Do you want to save them first?");
		if (retc == 0)
		{
			// "Cancel". Do not receive config
			return;
		}
		else if (retc == 1)
		{
			// "Yes". Try to save. Cancel if unsuccessful
			if (!saveSysExMapping())
				return;
		}
		// retc == 2: "No" -> no saving, overwrite
	}

	TerpstraSysExApplication::getApp().resetSysExMapping();

	// Request MIDI channel, MIDI note, colour and key type config for all keys
	getMidiDriver().sendGetCompleteMappingRequest();

	// General options
	// ToDo AfterTouchActive
	// ToDo LightOnKeyStrokes
	// ToDo invertFootController
	// ToDO expressionControllerSensivity

	// Velocity curve config
	getMidiDriver().sendVelocityIntervalConfigRequest();
	getMidiDriver().sendVelocityConfigurationRequest(TerpstraVelocityCurveConfig::VelocityCurveType::noteOnNoteOff);
	getMidiDriver().sendVelocityConfigurationRequest(TerpstraVelocityCurveConfig::VelocityCurveType::fader);
	getMidiDriver().sendVelocityConfigurationRequest(TerpstraVelocityCurveConfig::VelocityCurveType::afterTouch);
}

void TerpstraSysExApplication::updateMainTitle()
{
	String windowTitle("Lumatone Editor");
	if (!currentFile.getFileName().isEmpty())
		windowTitle << " - " << currentFile.getFileName();
	if (hasChangesToSave)
		windowTitle << "*";
	mainWindow->setName(windowTitle);
}

void TerpstraSysExApplication::setHasChangesToSave(bool value)
{
	if (value != hasChangesToSave)
	{
		hasChangesToSave = value;
		updateMainTitle();
	}
}

bool TerpstraSysExApplication::aboutTerpstraSysEx()
{
	String m;

	m << "Lumatone Editor" << newLine
		<< newLine
		<< "Version " << String((JUCE_APP_VERSION_HEX >> 16) & 0xff) << "."
		<< String((JUCE_APP_VERSION_HEX >> 8) & 0xff) << "."
		<< String(JUCE_APP_VERSION_HEX & 0xff) << newLine

		<< "@ Hans Straub, Vincenzo Sicurella 2014 - 2021" << newLine
		<< newLine
		<< "Based on the program 'TerpstraSysEx' @ Dylan Horvath 2007" << newLine
		<< newLine
		<< "For help on using this program, or any questions relating to the Lumatone keyboard, go to:" << newLine
		<< newLine
		<< "http://lumatone.io";

	DialogWindow::LaunchOptions options;
	Label* label = new Label();
	label->setLookAndFeel(&lookAndFeel);
	label->setText(m, dontSendNotification);
	options.content.setOwned(label);

	juce::Rectangle<int> area(0, 0, 400, 200);
	options.content->setSize(area.getWidth(), area.getHeight());

	resizeLabelWithHeight(label, roundToInt(area.getHeight() * 0.24f));

	options.dialogTitle = "About Lumatone Editor";
	options.dialogBackgroundColour = lookAndFeel.findColour(ResizableWindow::backgroundColourId);

	options.escapeKeyTriggersCloseButton = true;
	options.useNativeTitleBar = false;
	options.resizable = true;


	DialogWindow* dw = options.launchAsync();
	dw->centreWithSize(400, 260);

	return true;
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(TerpstraSysExApplication)
