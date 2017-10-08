#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"
#include "util_filesystem.h"

namespace MyGUI {

	class DropdownListButton;
	class FileDialog;

	typedef delegates::CMultiDelegate1<FileDialog*> EventHandle_FileDialogPtrFileDialog;

	class FileDialog : public wraps::BaseLayout {
	public:
		FileDialog();

		virtual ~FileDialog();

		/** must call this function before show */
		void initialize();

		/** Set smooth message showing*/
		FileDialog* setSmoothShow(bool _value);

		void endMessage();

		FileDialog* setMessageModal(bool _value);

		/** Event : button on message window pressed.\n
		signature : void method(FileDialog* _sender)\n
		@param _sender widget that called this event
		*/
		EventHandle_FileDialogPtrFileDialog
			eventFileDialogAccept;

		std::string currentDirectory;
		std::string rootDirectory;
		std::string fileName;
		std::vector<std::string> fileTypes;
		std::vector<std::string> fileExtensions;
		int selectedFileType;
		bool isSaveDialog;

	protected:
		void notifyButtonClick(Widget* _sender);

		void onKeyButtonPressed(Widget* _sender, KeyCode _key, Char _char);

		void _destroy(bool _result);

	private:
		void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);
		void notifyFolderButtonClick(Widget* _sender);
		void notifyFolderComboAccept(DropdownListButton* _sender, size_t _index);

		void refreshFileList();

		void compareFileList(MyGUI::MultiListBox* _sender, size_t _column, size_t _index1, size_t _index2, bool& _less);

		void selectLevel(int level);
		void selectSubFolder(int level, std::string subFolder);
		void recreatePathInfo();

		void addHistory();
		void prevHistory();
		void nextHistory();

		void updateButtonState();

	private:
		bool mSmoothShow;

		Button *cmdPrev, *cmdNext, *cmdUp;
		ScrollView *picFolder;
		MultiListBox *lstFile;
		EditBox *txtFileName;
		ComboBox *cmbFileType;

		std::vector<util::FileInfo> fileList;

		struct PathInfo {
			std::string name; // without path separator
			std::vector<util::FileInfo> subFolders;
			DropdownListButton *cmb;
		};

		std::vector<PathInfo> pathInfo;
		int selectedLevel;

		std::vector<std::string> history;
		int currentHistory;
		bool lockHistory;
	};

}
