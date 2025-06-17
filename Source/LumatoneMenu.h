/*
  ==============================================================================

    LumatoneMenu.h
    Created: 8 Jan 2021 7:42:59pm
    Author:  hsstraub

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "./data/LumatoneEditorState.h"

namespace Lumatone {
	namespace Menu {
		enum commandIDs {
			openSysExMapping = 0x200010,
			saveSysExMapping = 0x200011,
			saveSysExMappingAs = 0x200012,
			resetSysExMapping = 0x200013,
			importSysExMapping = 0x200014,

			selectAll = 0x200020,
			selectNone = 0x200021,

			deleteOctaveBoard = 0x200100,
			copyOctaveBoard = 0x200101,
			pasteOctaveBoard = 0x200102,
            pasteOctaveBoardNotes = 0x200103,
            pasteOctaveBoardChannels = 0x200104,
            pasteOctaveBoardColours = 0x200105,
            pasteOctaveBoardTypes = 0x200106,

			undo = 0x200200,
			redo = 0x200201,

			recentFilesBaseID = 0x200300,

			// Keyboard View Modes
			toggleKeyProperties = 0x200401,

			// colourViewSubMenu = 0x200500,
			colourViewRGB = 0x200501,
			colourViewModel = 0x200502,

			aboutSysEx = 0x300100
		};

		class MainMenuModel : public juce::MenuBarModel
							, public LumatoneEditorState
							, private LumatoneEditorState::Controller
		{
		public:
			MainMenuModel(const LumatoneEditorState& stateIn, ApplicationCommandManager* commandManager);

			virtual StringArray getMenuBarNames();
			PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);
			void menuItemSelected(int menuItemID, int topLevelMenuIndex);

			void createFileMenu(PopupMenu& menu);
			void createEditMenu(PopupMenu& menu);
			void createHelpMenu(PopupMenu& menu);
			void createColourViewSubMenu(PopupMenu& menu);

		private:
			ApplicationCommandManager * theManager;
		};

	}
	namespace Debug {
		enum commandIDs {
			toggleDeveloperMode = 0xA00001
		};
	}
}
