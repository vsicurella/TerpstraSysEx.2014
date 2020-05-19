/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.
	Created: xx.xx.2014

  ==============================================================================
*/

#include "Main.h"
#include "GeneralOptionsDlg.h"
#include "VelocityCurveDlg.h"

//==============================================================================

TerpstraSysExApplication::TerpstraSysExApplication()
	: tooltipWindow(), hasChangesToSave(false)
{
	PropertiesFile::Options options;
	options.applicationName = "LumatoneSetup";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Application Support";
	// XXX Folder name before rebranding: TerpstraSysEx
#if JUCE_LINUX
	options.folderName = "~/.config/LumatoneSetup";
#else
	options.folderName = "LumatoneSetup";
#endif
	propertiesFile = new PropertiesFile(options);
	jassert(propertiesFile != nullptr);

	int manufacturerId = propertiesFile->getIntValue("ManufacturerId", 0x002150);
	midiDriver.setManufacturerId(manufacturerId);

	switch (propertiesFile->getIntValue("ColourScheme"))
	{
	case 1:
		applyDarkColourScheme(false);
		break;

	default:
		applyLightColourScheme(false);
		break;
	}

	// Recent files list
	recentFiles.restoreFromString ( propertiesFile->getValue("RecentFiles") );
	recentFiles.removeNonExistentFiles();

	// State of main window will be read from properties file when main window is created
}

//==============================================================================
void TerpstraSysExApplication::initialise(const String& commandLine)
{
    // This method is where you should put your application's initialisation code..
	commandManager.reset(new ApplicationCommandManager());
	commandManager->registerAllCommandsForTarget(this);

	menuModel.reset(new TerpstraSysExMainMenuModel(commandManager.get()));

    mainWindow.reset(new MainWindow());
	mainWindow->setMenuBar(menuModel.get());
	mainWindow->addKeyListener(commandManager->getKeyMappings());

	((MainContentComponent*)(mainWindow->getContentComponent()))->restoreStateFromPropertiesFile(propertiesFile);

	// commandLine: may contain a file name
	if (!commandLine.isEmpty())
	{
		// commandLine is supposed to contain a file name. Try to open it.
		currentFile = File(commandLine);
		if (!currentFile.existsAsFile())
		{
			// If file name is with quotes, try removing the quotes
			if (commandLine.startsWithChar('"') && commandLine.endsWithChar('"'))
				currentFile = File(commandLine.substring(1, commandLine.length() - 1));
		}

		openFromCurrentFile();
	}
}

void TerpstraSysExApplication::shutdown()
{
    // Add your application's shutdown code here..

	// Save recent files list
	recentFiles.removeNonExistentFiles();
	jassert(propertiesFile != nullptr);
	propertiesFile->setValue("RecentFiles", recentFiles.toString());

	// Save state of main window
	((MainContentComponent*)(mainWindow->getContentComponent()))->saveStateToPropertiesFile(propertiesFile);

	propertiesFile->saveIfNeeded();
	delete propertiesFile;
	propertiesFile = nullptr;

    mainWindow = nullptr; // (deletes our window)
	menuModel = nullptr;
	commandManager = nullptr;
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
			// "Yes". Try to save. Canvel if unsuccessful
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

void TerpstraSysExApplication::getAllCommands(Array <CommandID>& commands)
{
	JUCEApplication::getAllCommands(commands);

	const CommandID ids[] = {
		TerpstraSysExMainMenuModel::commandIDs::openSysExMapping,
		TerpstraSysExMainMenuModel::commandIDs::saveSysExMapping,
		TerpstraSysExMainMenuModel::commandIDs::saveSysExMappingAs,
		TerpstraSysExMainMenuModel::commandIDs::resetSysExMapping,

		TerpstraSysExMainMenuModel::commandIDs::deleteOctaveBoard,
		TerpstraSysExMainMenuModel::commandIDs::copyOctaveBoard,
		TerpstraSysExMainMenuModel::commandIDs::pasteOctaveBoard,

		TerpstraSysExMainMenuModel::commandIDs::lightColourScheme,
		TerpstraSysExMainMenuModel::commandIDs::darkColourScheme,

		TerpstraSysExMainMenuModel::commandIDs::generalOptions,
		TerpstraSysExMainMenuModel::commandIDs::noteOnOffVelocityCurve,
		TerpstraSysExMainMenuModel::commandIDs::faderVelocityCurve,

		TerpstraSysExMainMenuModel::commandIDs::aboutSysEx
	};

	commands.addArray(ids, numElementsInArray(ids));
}

void TerpstraSysExApplication::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case TerpstraSysExMainMenuModel::commandIDs::openSysExMapping:
		result.setInfo("Load file mapping", "Open a Lumatone key mapping", "File", 0);
		result.addDefaultKeypress('o', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::saveSysExMapping:
		result.setInfo("Save mapping", "Save the current mapping to file", "File", 0);
		result.addDefaultKeypress('s', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::saveSysExMappingAs:
		result.setInfo("Save mapping as...", "Save the current mapping to new file", "File", 0);
		result.addDefaultKeypress('a', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::resetSysExMapping:
		result.setInfo("Reset", "Close file without saving and clear all edit fields", "File", 0);
		result.addDefaultKeypress('r', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::deleteOctaveBoard:
		result.setInfo("Delete", "Delete subboard data", "Edit", 0);
		result.addDefaultKeypress(KeyPress::deleteKey, ModifierKeys::noModifiers);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::copyOctaveBoard:
		result.setInfo("Copy", "Copy subboard data", "Edit", 0);
		result.addDefaultKeypress('c', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::pasteOctaveBoard:
		result.setInfo("Paste", "Paste subboard data", "Edit", 0);
		result.addDefaultKeypress('v', ModifierKeys::ctrlModifier);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::lightColourScheme:
		result.setInfo("Light", "Light colour scheme", "View", 0);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::darkColourScheme:
		result.setInfo("Dark", "Dark colour scheme", "View", 0);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::generalOptions:
		result.setInfo("General options", "General options", "Options", 0);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::noteOnOffVelocityCurve:
		result.setInfo("Note on/off velocity curve", "Note on/off velocity curve", "Options", 0);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::faderVelocityCurve:
		result.setInfo("Fader velocity curve", "Fader velocity curve", "Options", 0);
		break;

	case TerpstraSysExMainMenuModel::commandIDs::aboutSysEx:
		result.setInfo("About LumatoneSetup", "Shows version and copyright", "Help", 0);
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
	case TerpstraSysExMainMenuModel::commandIDs::openSysExMapping:
		return openSysExMapping();
	case TerpstraSysExMainMenuModel::commandIDs::saveSysExMapping:
		return saveSysExMapping();
	case TerpstraSysExMainMenuModel::commandIDs::saveSysExMappingAs:
		return saveSysExMappingAs();
	case TerpstraSysExMainMenuModel::commandIDs::resetSysExMapping:
		return resetSysExMapping();

	case TerpstraSysExMainMenuModel::commandIDs::deleteOctaveBoard:
		return deleteSubBoardData();
	case TerpstraSysExMainMenuModel::commandIDs::copyOctaveBoard:
		return copySubBoardData();
	case TerpstraSysExMainMenuModel::commandIDs::pasteOctaveBoard:
		return pasteSubBoardData();

	case TerpstraSysExMainMenuModel::commandIDs::lightColourScheme:
		return applyLightColourScheme(true);
	case TerpstraSysExMainMenuModel::commandIDs::darkColourScheme:
		return applyDarkColourScheme(true);

	case TerpstraSysExMainMenuModel::commandIDs::generalOptions:
		return generalOptionsDialog();
	case TerpstraSysExMainMenuModel::commandIDs::noteOnOffVelocityCurve:
		return noteOnOffVelocityCurveDialog();
	case TerpstraSysExMainMenuModel::commandIDs::faderVelocityCurve:
		return faderVelocityCurveDialog();

	case TerpstraSysExMainMenuModel::commandIDs::aboutSysEx:
		return aboutTerpstraSysEx();
	default:
		return JUCEApplication::perform(info);
	}
}

bool TerpstraSysExApplication::openSysExMapping()
{
	// XXX If there are changes: ask for saving these first?

	FileChooser chooser("Open a Lumatone key mapping", File(), "*.tsx");
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
	FileChooser chooser("Lumatone Key Mapping Files", File(), "*.tsx");
	if (chooser.browseForFileToSave(true))
	{
		currentFile = chooser.getResult();
		if (saveCurrentFile() )
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
	TerpstraKeyMapping keyMapping;
	((MainContentComponent*)(mainWindow->getContentComponent()))->setData(keyMapping);

	setHasChangesToSave(false);

	// Window title
	updateMainTitle();

	return true;
}

bool TerpstraSysExApplication::deleteSubBoardData()
{
	return ((MainContentComponent*)(mainWindow->getContentComponent()))->deleteCurrentSubBoardData();
}

bool TerpstraSysExApplication::copySubBoardData()
{
	return ((MainContentComponent*)(mainWindow->getContentComponent()))->copyCurrentSubBoardData();
}

bool TerpstraSysExApplication::pasteSubBoardData()
{
	return ((MainContentComponent*)(mainWindow->getContentComponent()))->pasteCurrentSubBoardData();
}

bool TerpstraSysExApplication::applyLightColourScheme(bool repaintAndSave)
{
	Colour windowBackgroundColour(0xffbad0de);
	Colour editFieldBackgroundColour = Colours::white;
	Colour textColour(0xff000000);

	lookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, windowBackgroundColour);

	lookAndFeel.setColour(juce::DocumentWindow::backgroundColourId, windowBackgroundColour);

	lookAndFeel.setColour(juce::TextEditor::backgroundColourId, editFieldBackgroundColour);
	lookAndFeel.setColour(juce::TextEditor::textColourId, textColour);
	lookAndFeel.setColour(juce::TextEditor::highlightColourId, Colour(0xffc3c3fa));

	lookAndFeel.setColour(juce::ComboBox::backgroundColourId, editFieldBackgroundColour);
	lookAndFeel.setColour(juce::ComboBox::textColourId, textColour);
	lookAndFeel.setColour(juce::ComboBox::arrowColourId, textColour);

	lookAndFeel.setColour(juce::GroupComponent::outlineColourId, Colour(0x66000000));
	lookAndFeel.setColour(juce::GroupComponent::textColourId, textColour);

	lookAndFeel.setColour(juce::Label::textColourId, textColour);

	lookAndFeel.setColour(juce::ToggleButton::textColourId, textColour);
	lookAndFeel.setColour(juce::ToggleButton::tickColourId, textColour);

	lookAndFeel.setColour(TerpstraKeyEdit::backgroundColourId, windowBackgroundColour);
	lookAndFeel.setColour(TerpstraKeyEdit::outlineColourId, Colours::black);
	lookAndFeel.setColour(TerpstraKeyEdit::selectedKeyOutlineId, Colour(0xfff7990d));

	lookAndFeel.setColour(VelocityCurveBeam::beamColourId, Colour(0x66ff5e00));
	lookAndFeel.setColour(VelocityCurveBeam::outlineColourId, Colours::black);

	if (repaintAndSave)
	{
		mainWindow->setBackgroundColour(windowBackgroundColour);
		mainWindow->repaint();

		// Save the choice in settings file
		propertiesFile->setValue("ColourScheme", 0);
	}

	return true;
}

bool TerpstraSysExApplication::applyDarkColourScheme(bool repaintAndSave)
{
	Colour windowBackgroundColour(0xff373737);
	Colour editFieldBackgroundColour(0xff2f2f2f);
	Colour textColour(0xffd7d9da);

	lookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, windowBackgroundColour);

	lookAndFeel.setColour(juce::DocumentWindow::backgroundColourId, windowBackgroundColour);

	lookAndFeel.setColour(juce::TextEditor::backgroundColourId, editFieldBackgroundColour);
	lookAndFeel.setColour(juce::TextEditor::textColourId, textColour);
	lookAndFeel.setColour(juce::TextEditor::highlightColourId, Colour(0xffc3c3fa));

	lookAndFeel.setColour(juce::ComboBox::backgroundColourId, editFieldBackgroundColour);
	lookAndFeel.setColour(juce::ComboBox::textColourId, textColour);
	lookAndFeel.setColour(juce::ComboBox::arrowColourId, Colour(0xfff7990d));

	lookAndFeel.setColour(juce::Label::textColourId, textColour);

	lookAndFeel.setColour(juce::ToggleButton::textColourId, textColour);
	lookAndFeel.setColour(juce::ToggleButton::tickColourId, Colour(0xfff7990d));

	lookAndFeel.setColour(juce::GroupComponent::outlineColourId, textColour);
	lookAndFeel.setColour(juce::GroupComponent::textColourId, textColour);

	lookAndFeel.setColour(TerpstraKeyEdit::backgroundColourId, windowBackgroundColour);
	lookAndFeel.setColour(TerpstraKeyEdit::outlineColourId, Colour(0xffd7d9da));
	lookAndFeel.setColour(TerpstraKeyEdit::selectedKeyOutlineId, Colour(0xfff7990d));

	lookAndFeel.setColour(VelocityCurveBeam::beamColourId, Colour(0x66ff5e00));
	lookAndFeel.setColour(VelocityCurveBeam::outlineColourId, Colour(0xffd7d9da));

	if (repaintAndSave)
	{
		mainWindow->setBackgroundColour(windowBackgroundColour);
		mainWindow->repaint();

		// Save the choice in settings file
		propertiesFile->setValue("ColourScheme", 1);
	}

	return true;
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
	VelocityCurveDlg* optionsWindow = new VelocityCurveDlg(TerpstraMidiDriver::VelocityCurveType::noteOnNoteOff);
	optionsWindow->setLookAndFeel(&lookAndFeel);

	int dlgWidth = propertiesFile->getIntValue("VelocityCurveWindowWidth", 640);
	int dlgHeight = propertiesFile->getIntValue("VelocityCurveWindowHeight", 320);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(optionsWindow);
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
	VelocityCurveDlg* optionsWindow = new VelocityCurveDlg(TerpstraMidiDriver::VelocityCurveType::fader);
	optionsWindow->setLookAndFeel(&lookAndFeel);

	int dlgWidth = propertiesFile->getIntValue("VelocityCurveWindowWidth", 640);
	int dlgHeight = propertiesFile->getIntValue("VelocityCurveWindowHeight", 320);

	DialogWindow::LaunchOptions launchOptions;
	launchOptions.content.setOwned(optionsWindow);
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

		// Mark file as unchanged
		setHasChangesToSave(false);

		// Window title
		updateMainTitle();

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

	// Add file to recent files list - or put it on top of the list
	recentFiles.addFile(currentFile);

	return retc;
}

void TerpstraSysExApplication::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
	((MainContentComponent*)(mainWindow->getContentComponent()))->handleIncomingMidiMessage(source, message);
}

void TerpstraSysExApplication::sendCurrentMappingToDevice()
{
    getMidiDriver().sendCompleteMapping(
        ((MainContentComponent*)(mainWindow->getContentComponent()))->getMappingInEdit()
    );
}

void TerpstraSysExApplication::updateMainTitle()
{
	String windowTitle("Lumatone Setup Utility");
	if (!currentFile.getFileName().isEmpty() )
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

	m << "Lumatone Keyboard Setup Utility" << newLine
		<< newLine
		<< "Version " << String((JUCE_APP_VERSION_HEX >> 16) & 0xff) << "."
		<< String((JUCE_APP_VERSION_HEX >> 8) & 0xff) << "."
		<< String(JUCE_APP_VERSION_HEX & 0xff) << newLine

		<< newLine
		<< "Original design @ Dylan Horvath 2007" << newLine
		<< "Reengineered @ Hans Straub 2014 - 2020" << newLine
		<< newLine
		<< "For help on using this program, or any questions relating to the Lumatone keyboard, go to http://lumatone.io or http://terpstrakeyboard.com .";

	DialogWindow::LaunchOptions options;
	Label* label = new Label();
	label->setLookAndFeel(&lookAndFeel);
	label->setText(m, dontSendNotification);
	options.content.setOwned(label);

	Rectangle<int> area(0, 0, 400, 200);
	options.content->setSize(area.getWidth(), area.getHeight());

	options.dialogTitle = "About LumatoneSetup";
	options.dialogBackgroundColour = lookAndFeel.findColour(ResizableWindow::backgroundColourId);

	options.escapeKeyTriggersCloseButton = true;
	options.useNativeTitleBar = false;
	options.resizable = true;

	//const RectanglePlacement placement(RectanglePlacement::xRight + RectanglePlacement::yBottom + RectanglePlacement::doNotResize);

	DialogWindow* dw = options.launchAsync();
	dw->centreWithSize(400, 260);

	return true;
}


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (TerpstraSysExApplication)
