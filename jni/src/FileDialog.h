#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"
#include "util_filesystem.h"

namespace MyGUI {

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

		void refreshFileList();

	private:
		bool mSmoothShow;

		Button *cmdPrev, *cmdNext, *cmdUp, *cmdFolder;
		MultiListBox *lstFile;
		EditBox *txtFileName;
		ComboBox *cmbFileType;

		std::vector<util::FileInfo> fileList;
	};

}
