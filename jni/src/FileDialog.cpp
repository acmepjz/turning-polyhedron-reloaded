#include "FileDialog.h"
#include "util_filesystem.h"

#include <osgDB/FileUtils>

#include <stdio.h>

namespace MyGUI {

	FileDialog::FileDialog() :
		wraps::BaseLayout("FileDialog.layout"),
		mSmoothShow(false),
		selectedFileType(-1),
		isSaveDialog(false)
	{
	}

	void FileDialog::initialize() {
		Window *window = dynamic_cast<Window*>(mMainWidget);

		window->eventWindowButtonPressed += newDelegate(this, &FileDialog::notifyWindowButtonPressed);

		Widget *temp;

		assignWidget(cmdPrev, "cmdPrev", false);
		cmdPrev->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		assignWidget(cmdNext, "cmdNext", false);
		cmdNext->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		assignWidget(cmdUp, "cmdUp", false);
		cmdUp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		assignWidget(cmdFolder, "cmdFolder", false);
		cmdFolder->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);

		if (currentDirectory.empty()) currentDirectory = osgDB::getCurrentWorkingDirectory();
		cmdFolder->setCaption(currentDirectory);

		assignWidget(lstFile, "lstFile", false);
		lstFile->addColumn("File name");
		lstFile->addColumn("Modify time", 128);
		lstFile->addColumn("Extension", 80);
		lstFile->addColumn("Size", 96);
		lstFile->setColumnResizingPolicyAt(0, ResizingPolicy::Fill);
		lstFile->setColumnResizingPolicyAt(1, ResizingPolicy::Fixed);
		lstFile->setColumnResizingPolicyAt(2, ResizingPolicy::Fixed);
		lstFile->setColumnResizingPolicyAt(3, ResizingPolicy::Fixed);
		lstFile->setColumnSkinLineAt(0, "ListBoxItemL");
		lstFile->setColumnSkinLineAt(1, "ListBoxItemM");
		lstFile->setColumnSkinLineAt(2, "ListBoxItemM");
		lstFile->setColumnSkinLineAt(3, "ListBoxItemR_Right");

		lstFile->requestOperatorLess2 = newDelegate(this, &FileDialog::compareFileList);

		assignWidget(txtFileName, "txtFileName", false);
		txtFileName->setCaption(fileName);

		assignWidget(cmbFileType, "cmbFileType", false);
		{
			const int m = fileTypes.size();
			for (int i = 0; i < m; i++) {
				cmbFileType->addItem(fileTypes[i]);
			}
			if (selectedFileType >= 0 && selectedFileType < m) {
				cmbFileType->setIndexSelected(selectedFileType);
			} else {
				selectedFileType = -1;
			}
		}

		assignWidget(temp, "cmdNewFolder", false);
		temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		assignWidget(temp, "cmdOK", false);
		temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		assignWidget(temp, "cmdCancel", false);
		temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);

		refreshFileList();
	}

	void FileDialog::refreshFileList() {
		lstFile->removeAllItems();

		if (selectedFileType >= 0 && selectedFileType < (int)fileExtensions.size() && !fileExtensions[selectedFileType].empty()) {
			util::enumAllFiles(fileList, currentDirectory, fileExtensions[selectedFileType].c_str(), true, true, false);
		} else {
			util::enumAllFiles(fileList, currentDirectory, NULL, true, true, false);
		}

		char s[32];
		const char* s1[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", NULL };

		for (int i = 0, m = fileList.size(); i < m; i++) {
			lstFile->addItem(fileList[i].name);
			lstFile->setSubItemNameAt(1, i, fileList[i].mtime);
			if (fileList[i].isFolder) {
				lstFile->setSubItemNameAt(2, i, "Folder");
			} else {
				lstFile->setSubItemNameAt(2, i, fileList[i].ext);

				long long size = fileList[i].size;
				for (int j = 0; s1[j]; j++) {
					if (size < 1000) {
						sprintf(s, "%d ", int(size));
						lstFile->setSubItemNameAt(3, i, std::string(s) + s1[j]);
						break;
					} else if (size < 1000000) {
						sprintf(s, "%d,%03d ", int(size) / 1000, int(size) % 1000);
						lstFile->setSubItemNameAt(3, i, std::string(s) + s1[j]);
						break;
					}
					size = (size + 512) >> 10;
				}
			}
		}
	}

	void FileDialog::compareFileList(MyGUI::MultiListBox* _sender, size_t _column, size_t _index1, size_t _index2, bool& _less) {
		switch (_column) {
		case 1: // sort by time
			_less = util::FileInfoComparer::compare(util::FileInfoComparer::SortByTime, false, fileList[_index1], fileList[_index2]) < 0;
			break;
		case 2: // sort by extension
			_less = util::FileInfoComparer::compare(util::FileInfoComparer::SortByExtension, false, fileList[_index1], fileList[_index2]) < 0;
			break;
		case 3: // sort by size
			_less = util::FileInfoComparer::compare(util::FileInfoComparer::SortBySize, false, fileList[_index1], fileList[_index2]) < 0;
			break;
		default: // sort by name
			_less = util::FileInfoComparer::compare(util::FileInfoComparer::SortByName, false, fileList[_index1], fileList[_index2]) < 0;
			break;
		}
	}

	FileDialog::~FileDialog() {
	}

	FileDialog* FileDialog::setSmoothShow(bool _value)
	{
		mSmoothShow = _value;
		if (mSmoothShow)
		{
			mMainWidget->setAlpha(ALPHA_MIN);
			mMainWidget->setVisible(true);
			mMainWidget->castType<Window>()->setVisibleSmooth(true);
		}
		return this;
	}

	FileDialog* FileDialog::setMessageModal(bool _value)
	{
		if (_value)
			InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			InputManager::getInstance().removeWidgetModal(mMainWidget);
		return this;
	}

	void FileDialog::endMessage() {
		_destroy(false);
	}

	void FileDialog::notifyButtonClick(Widget* _sender) {
		std::string _name = _sender->getName().substr(mPrefix.size()); // ???

		if (_name == "cmdCancel") {
			endMessage();
		}
	}

	void FileDialog::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name) {
		if (_name == "close") {
			endMessage();
		}
	}

	void FileDialog::_destroy(bool _result) {
		if (_result) eventFileDialogAccept(this);

		delete this;
	}

}
