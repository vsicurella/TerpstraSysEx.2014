/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.
	Created: xx.xx.2014

  ==============================================================================
*/

#include "Main.h"


//==============================================================================

TerpstraSysExApplication::TerpstraSysExApplication() 
	: tooltipWindow(), hasChangesToSave(false)
{
	PropertiesFile::Options options;
	options.applicationName = "TerpstraSysEx";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
	options.folderName = "~/.config/TerpstraSysEx";
#else
	options.folderName = "TerpstraSysEx";
#endif
	propertiesFile = new PropertiesFile(options);
	jassert(propertiesFile != nullptr);

	// Recent files list
	recentFiles.restoreFromString ( propertiesFile->getValue("RecentFiles") );
	recentFiles.removeNonExistentFiles();

	// State of main window will be read form properties file when main window is created
}

//==============================================================================
void TerpstraSysExApplication::initialise(const String& commandLine)
{
    // This method is where you should put your application's initialisation code..   
	commandManager = new ApplicationCommandManager();
	commandManager->registerAllCommandsForTarget(this);

	menuModel = new TerpstraSysExMainMenuModel(commandManager);

    mainWindow = new MainWindow();
	mainWindow->setMenuBar(menuModel);
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
		TerpstraSysExMainMenuModel::commandIDs::aboutSysEx
	};

	commands.addArray(ids, numElementsInArray(ids));
}

void TerpstraSysExApplication::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case TerpstraSysExMainMenuModel::commandIDs::openSysExMapping:
		result.setInfo("Load file mapping", "Open a Terpstra SysEx mapping", "File", 0);
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

	case TerpstraSysExMainMenuModel::commandIDs::aboutSysEx:
		result.setInfo("About TerpstraSysEx", "Shows version and copyright", "Help", 0);
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
	case TerpstraSysExMainMenuModel::commandIDs::aboutSysEx:
		return aboutTerpstraSysEx();
	default:                        
		return JUCEApplication::perform(info);
	}
}

bool TerpstraSysExApplication::openSysExMapping()
{
	// XXX If there are changes: ask for saving these first? 

	FileChooser chooser("Open a Terpstra SysEx mapping", File::nonexistent, "*.tsx");
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
	FileChooser chooser("Terpstra SysEx Key Mapping Files", File::nonexistent, "*.tsx");
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

void TerpstraSysExApplication::updateMainTitle()
{
	String windowTitle("Terpstra Keyboard SysEx Utility");
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

	// XXX Version: there are the internal constants JUCE_APP_VERSION and JUCE_APP_VERSION_HEX...
	m << "Terpstra SysEx Utility" << newLine
		<< newLine
		<< "Version 0.4.1"	<< newLine
		<< newLine
		<< "Original design @ Dylan Horvath 2007" << newLine
		<< "Reengineered @ Hans Straub 2014 - 2018" << newLine
		<< "Program icon based on a design by Bo Constantinsen" << newLine
		<< newLine
		<< "For help on using this program, or any questions relating to the Terpstra keyboard, go to http://terpstrakeyboard.com .";

	DialogWindow::LaunchOptions options;
	Label* label = new Label();
	label->setText(m, dontSendNotification);
	options.content.setOwned(label);

	Rectangle<int> area(0, 0, 400, 240);
	options.content->setSize(area.getWidth(), area.getHeight());

	options.dialogTitle = "About TerpstraSysEx";
	options.dialogBackgroundColour = Colour(MAINWINDOWBGCOLOUR);
	options.escapeKeyTriggersCloseButton = true;
	options.useNativeTitleBar = false;
	options.resizable = true;

	const RectanglePlacement placement(RectanglePlacement::xRight + RectanglePlacement::yBottom + RectanglePlacement::doNotResize);

	DialogWindow* dw = options.launchAsync();
	dw->centreWithSize(400, 300);

	return true;
}


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (TerpstraSysExApplication)
