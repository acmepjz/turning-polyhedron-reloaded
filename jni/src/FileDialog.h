#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"
#include "MessageBox.h"
#include "MYGUIAccelerator.h"
#include "util_filesystem.h"

namespace MyGUI {

	class DropdownListButton;
	class FileDialog;
	class InputBox;

	typedef delegates::CMultiDelegate1<FileDialog*> EventHandle_FileDialogPtrFileDialog;

	class FileDialog : public wraps::BaseLayout {
	public:
		FileDialog();

		virtual ~FileDialog();

		/** must call this function before show */
		void initialize();

		void endMessage();

		/** Event : button on message window pressed.\n
		signature : void method(FileDialog* _sender)\n
		@param _sender widget that called this event
		*/
		EventHandle_FileDialogPtrFileDialog
			eventFileDialogAccept;

		std::string currentDirectory;
		std::string rootDirectory; // still unimplemented
		std::string fileName;
		std::vector<std::string> fileTypes;
		std::vector<std::string> fileExtensions;
		int selectedFileType;
		bool isSaveDialog;

		/** add a file type
		\param type a string description of the file type
		\param extension the extensions of the file type, separated by space and omit the first dot, e.g. "zip tar.gz"
		*/
		void addFileType(const std::string& type, const std::string& extension) {
			fileTypes.push_back(type);
			fileExtensions.push_back(extension);
		}

	protected:
		void notifyButtonClick(Widget* _sender);

		void _destroy(bool _result);

	private:
		void notifyAcceleratorKeyPressed(MYGUIAccelerator* sender, MyGUI::Widget* widget);
		void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);
		void notifyFolderButtonClick(Widget* _sender);
		void notifyFolderComboAccept(DropdownListButton* _sender, size_t _index);
		void notifyListSelectAccept(MultiListBox* _sender, size_t _position);
		void notifyListMouseItemActivate(MultiListBox* _sender, size_t _position);
		void notifyFilterComboAccept(ComboBox* _sender, size_t _index);
		void notifyOverwritePrompt(Message* sender, MessageBoxStyle result);
		void notifyInputFolder(InputBox* _sender);
		void notifyNewFolder(InputBox* _sender);

		virtual void frameEntered(float _frame) override;

		void cmdOK_Click();

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

		MYGUIAccelerator _accel;
	};

}
