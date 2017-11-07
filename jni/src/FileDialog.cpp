#include "FileDialog.h"
#include "util_filesystem.h"
#include "DropdownListButton.h"
#include "MessageBox.h"
#include "InputBox.h"

#include <osgGA/GUIEventAdapter>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>

#include <stdio.h>

namespace MyGUI {

	FileDialog::FileDialog() :
		wraps::BaseLayout("FileDialog.layout"),
		selectedFileType(0),
		selectedLevel(-1),
		currentHistory(-1),
		lockHistory(false),
		isSaveDialog(false)
	{
	}

	void FileDialog::notifyAcceleratorKeyPressed(MYGUIAccelerator* sender, MyGUI::Widget* widget) {
		// the operation must be delayed otherwise it crashes
		setFrameAdvise((int)(intptr_t)widget);
	}

	void FileDialog::frameEntered(float _frame) {
		int fa = getFrameAdvise();
		setFrameAdvise(0);

		if (fa & 0x10) {
			// enter
			cmdOK_Click();
		} else {
			// escape
			endMessage();
		}
	}

	void FileDialog::initialize() {
		Window *window = dynamic_cast<Window*>(mMainWidget);

		_accel.rootWidget = mMainWidget;
		_accel.addAccelerator((MyGUI::Widget*)0x100, true, osgGA::GUIEventAdapter::KEY_Escape, 0);
		_accel.addAccelerator((MyGUI::Widget*)0x110, true, osgGA::GUIEventAdapter::KEY_Return, 0);
		_accel.addAccelerator((MyGUI::Widget*)0x111, true, osgGA::GUIEventAdapter::KEY_KP_Enter, 0);
		_accel.eventAcceleratorKeyPressed += MyGUI::newDelegate(this, &FileDialog::notifyAcceleratorKeyPressed);

		if (isSaveDialog) window->setCaption("Save as");
		else window->setCaption("Open");

		window->eventWindowButtonPressed += newDelegate(this, &FileDialog::notifyWindowButtonPressed);

		ASSIGN_WIDGET0(cmdPrev);
		cmdPrev->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		ASSIGN_WIDGET0(cmdNext);
		cmdNext->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		ASSIGN_WIDGET0(cmdUp);
		cmdUp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);

		ASSIGN_WIDGET0(picFolder);

		ASSIGN_WIDGET0(lstFile);
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

		lstFile->sortByColumn(0);

		lstFile->eventListMouseItemActivate += newDelegate(this, &FileDialog::notifyListMouseItemActivate);
		lstFile->eventListSelectAccept += newDelegate(this, &FileDialog::notifyListSelectAccept);

		ASSIGN_WIDGET0(txtFileName);
		txtFileName->setOnlyText(fileName);

		ASSIGN_WIDGET0(cmbFileType);
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
		cmbFileType->eventComboAccept += newDelegate(this, &FileDialog::notifyFilterComboAccept);

		{
			Button *temp;
			assignWidget(temp, "cmdFolderSwitch", false);
			temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
			assignWidget(temp, "cmdNewFolder", false);
			temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
			assignWidget(temp, "cmdOK", false);
			temp->setCaption(isSaveDialog ? "Save" : "Open");
			temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
			assignWidget(temp, "cmdCancel", false);
			temp->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
		}

		if (currentDirectory.empty()) currentDirectory = osgDB::getCurrentWorkingDirectory();
		recreatePathInfo();

		if (!fileName.empty()) {
			for (int i = 0, m = fileList.size(); i < m; i++) {
				if (fileList[i].name == fileName) {
					lstFile->setIndexSelected(i);
					lstFile->beginToItemSelected();
					break;
				}
			}
		}
	}

	void FileDialog::notifyFolderButtonClick(Widget* _sender) {
		selectLevel(*_sender->getUserData<int>());
	}

	void FileDialog::selectLevel(int level) {
		int m = pathInfo.size();

		if (level >= 0 && level < m - 1 && level != selectedLevel) {
			selectedLevel = level;

			for (int i = 0; i < m; i++) {
				pathInfo[i].cmb->setStateSelected(i == level);
			}

			currentDirectory.clear();
			for (int i = 0; i <= level; i++) {
				currentDirectory += pathInfo[i].name + "/";
			}

			addHistory();
			refreshFileList();
		}
	}

	void FileDialog::updateButtonState() {
		cmdPrev->setEnabled(!history.empty() && currentHistory > 0);
		cmdNext->setEnabled(!history.empty() && currentHistory < (int)history.size() - 1);
		cmdUp->setEnabled(selectedLevel > 0);
	}

	void FileDialog::notifyFolderComboAccept(DropdownListButton* _sender, size_t _index) {
		int level = *_sender->getUserData<int>();
		if (level >= 0 && level < (int)pathInfo.size()) {
			if (_index < pathInfo[level].subFolders.size()) {
				selectSubFolder(level, pathInfo[level].subFolders[_index].name);
			}
		}
	}

	void FileDialog::recreatePathInfo() {
		for (int i = 0, m = pathInfo.size(); i < m; i++) {
			Gui::getInstance().destroyWidget(pathInfo[i].cmb);
		}

		pathInfo.clear();

		currentDirectory = osgDB::convertFileNameToNativeStyle(currentDirectory);
		if (!osgDB::isAbsolutePath(currentDirectory)) {
			currentDirectory = osgDB::concatPaths(osgDB::getCurrentWorkingDirectory(), currentDirectory);
		}
		currentDirectory = osgDB::convertFileNameToUnixStyle(currentDirectory);

#ifdef WIN32
		if (currentDirectory.size() < 3) {
			currentDirectory = "C:/";
		} else {
			char c = currentDirectory[0];
			if (c >= 'a' && c <= 'z') {
				c += ('A' - 'a');
				currentDirectory[0] = c;
			}
			if (c < 'A' || c > 'Z' || currentDirectory[1] != ':' || currentDirectory[2] != '/') {
				currentDirectory = "C:/";
			}
		}
#else
		if (currentDirectory.empty() || currentDirectory[0] != '/') currentDirectory = "/";
#endif

		std::vector<std::string> paths;
		size_t lps = 0;
		for (;;) {
			size_t lpe = currentDirectory.find_first_of('/', lps);
			std::string s = osgDB::trimEnclosingSpaces(currentDirectory.substr(lps, (lpe == std::string::npos) ? lpe : lpe - lps));
			if (paths.empty()) {
				paths.push_back(s);
			} else if (!s.empty() && s != ".") {
				if (s == "..") {
					if (paths.size() > 1) paths.pop_back();
				} else {
					paths.push_back(s);
				}
			}
			if (lpe == std::string::npos) break;
			lps = lpe + 1;
		}

		const int m = paths.size();
		selectedLevel = m - 1;

		pathInfo.resize(m + 1);

		currentDirectory.clear();
		int left = 0;

		for (int i = 0; i <= m; i++) {
			if (i < m) pathInfo[i].name = paths[i];

			DropdownListButton *cmb = picFolder->createWidgetT("DropdownListButton", "DropdownListButton",
				IntCoord(left, 0, 128, 24), Align::Default)->castType<DropdownListButton>();
			cmb->setAutoCaption(false);
			cmb->setAutoSize(true);
			cmb->setListWidth(400);
			cmb->setUserData(Any(i));
			if (i >= m) {
				cmb->setCaption("");
			} else {
				cmb->setCaption(paths[i]);
				cmb->setStateSelected(i == selectedLevel);
			}
			left += cmb->getWidth();

			if (i == 0) {
				std::vector<util::DriverInfo> drivers;
				util::enumAllDrivers(drivers);
				const int n = drivers.size();
				pathInfo[i].subFolders.resize(n);
				for (int j = 0; j < n; j++) {
					pathInfo[i].subFolders[j].name = drivers[j].name;
					cmb->addItem(drivers[j].displayName);
				}
			} else {
				currentDirectory += paths[i - 1] + "/";
				util::enumAllFiles(pathInfo[i].subFolders, currentDirectory, NULL, false, true, false);
				for (int j = 0, n = pathInfo[i].subFolders.size(); j < n; j++) {
					cmb->addItem(pathInfo[i].subFolders[j].name);
				}
			}

			pathInfo[i].cmb = cmb;

			cmb->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyFolderButtonClick);
			cmb->eventComboAccept += newDelegate(this, &FileDialog::notifyFolderComboAccept);
		}

		picFolder->setCanvasSize(IntSize(left, 24));

		addHistory();
		refreshFileList();
	}

	void FileDialog::selectSubFolder(int level, std::string subFolder) {
		while ((int)pathInfo.size() > level + 1) {
			Gui::getInstance().destroyWidget(pathInfo.back().cmb);
			pathInfo.pop_back();
		}

		pathInfo[level].name = subFolder;
		pathInfo[level].cmb->setCaption(subFolder);

		currentDirectory.clear();
		for (int i = 0; i <= level; i++) {
			currentDirectory += pathInfo[i].name + "/";
			pathInfo[i].cmb->setStateSelected(i == level);
		}

		pathInfo.push_back(PathInfo());

		int left = pathInfo[level].cmb->getRight();
		DropdownListButton *cmb = picFolder->createWidgetT("DropdownListButton", "DropdownListButton",
			IntCoord(left, 0, 128, 24), Align::Default)->castType<DropdownListButton>();
		cmb->setAutoCaption(false);
		cmb->setAutoSize(true);
		cmb->setListWidth(400);
		cmb->setUserData(Any(level + 1));
		cmb->setCaption("");

		util::enumAllFiles(pathInfo[level + 1].subFolders, currentDirectory, NULL, false, true, false);
		for (int j = 0, n = pathInfo[level + 1].subFolders.size(); j < n; j++) {
			cmb->addItem(pathInfo[level + 1].subFolders[j].name);
		}

		pathInfo[level + 1].cmb = cmb;

		cmb->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyFolderButtonClick);
		cmb->eventComboAccept += newDelegate(this, &FileDialog::notifyFolderComboAccept);

		picFolder->setCanvasSize(IntSize(left + cmb->getWidth(), 24));

		selectedLevel = level;

		addHistory();
		refreshFileList();
	}

	void FileDialog::addHistory() {
		if (!lockHistory) {
			while ((int)history.size() > currentHistory + 1) {
				history.pop_back();
			}
			history.push_back(currentDirectory);
			currentHistory = history.size() - 1;
		}

		updateButtonState();
	}

	void FileDialog::prevHistory() {
		if (currentHistory > 0 && currentHistory < (int)history.size()) {
			lockHistory = true;
			currentHistory--;
			currentDirectory = history[currentHistory];
			recreatePathInfo();
			lockHistory = false;
		}
	}

	void FileDialog::nextHistory() {
		if (currentHistory >= 0 && currentHistory < (int)history.size() - 1) {
			lockHistory = true;
			currentHistory++;
			currentDirectory = history[currentHistory];
			recreatePathInfo();
			lockHistory = false;
		}
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

	void FileDialog::endMessage() {
		_destroy(false);
	}

	void FileDialog::notifyButtonClick(Widget* _sender) {
		std::string _name = removePrefix(_sender->getName());

		if (_name == "cmdCancel") {
			endMessage();
		} else if (_name == "cmdPrev") {
			prevHistory();
		} else if (_name == "cmdNext") {
			nextHistory();
		} else if (_name == "cmdUp") {
			selectLevel(selectedLevel - 1);
		} else if (_name == "cmdFolderSwitch") {
			InputBox::createInputBox("Input path", "Input path",
				currentDirectory)->eventInputBoxAccept += newDelegate(this, &FileDialog::notifyInputFolder);
		} else if (_name == "cmdNewFolder") {
			InputBox::createInputBox("New folder", "Input the name of the new folder")->eventInputBoxAccept += newDelegate(this, &FileDialog::notifyNewFolder);
		} else if (_name == "cmdOK") {
			cmdOK_Click();
		}
	}

	void FileDialog::notifyInputFolder(InputBox* _sender) {
		std::string s = osgDB::trimEnclosingSpaces(_sender->getText());
		if (!s.empty()) {
			currentDirectory = s;
			recreatePathInfo();
		}
	}

	void FileDialog::notifyNewFolder(InputBox* _sender) {
		std::string s = osgDB::trimEnclosingSpaces(_sender->getText());
		if (!s.empty()) {
			if (osgDB::makeDirectory(currentDirectory + s)) {
				refreshFileList();
			} else {
				Message::createMessageBox("Error",
					"Failed to create folder '" + s + "'.",
					MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			}
		}
	}

	void FileDialog::cmdOK_Click() {
		fileName = osgDB::trimEnclosingSpaces(txtFileName->getOnlyText());

		if (fileName.empty() || fileName == ".") return;
		if (fileName == "..") {
			selectLevel(selectedLevel - 1);
			return;
		}

		// check if the file name is valid
		if (fileName.find_first_of("/\\:*?\"<>|") != std::string::npos) {
			Message::createMessageBox(mMainWidget->castType<Window>()->getCaption(),
				"The file name is invalid.",
				MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;
		}

		// check if it is a directory
		osgDB::FileType type = osgDB::fileType(currentDirectory + fileName);
		if (type == osgDB::DIRECTORY) {
			selectSubFolder(selectedLevel + 1, fileName);
			return;
		}

		// add default extension
		if ((isSaveDialog || type == osgDB::FILE_NOT_FOUND) && fileName.find_first_of('.') == std::string::npos) {
			if (selectedFileType >= 0 && selectedFileType < (int)fileExtensions.size()) {
				std::string s = osgDB::trimEnclosingSpaces(fileExtensions[selectedFileType]);
				s = s.substr(0, s.find_first_of(' '));
				if (!s.empty()) {
					fileName += "." + s;
					txtFileName->setOnlyText(fileName);
					type = osgDB::fileType(currentDirectory + fileName);
					if (type == osgDB::DIRECTORY) {
						selectSubFolder(selectedLevel + 1, fileName);
						return;
					}
				}
			}
		}

		// check if it already exists
		if (type == osgDB::REGULAR_FILE) {
			if (isSaveDialog) {
				Message::createMessageBox("Save as",
					"The file '" + fileName + "' already exists. Do you want to overwrite it?",
					MessageBoxStyle::IconWarning | MessageBoxStyle::YesNo)->setCancelButton(MessageBoxStyle::No)->eventMessageBoxResult += newDelegate(this, &FileDialog::notifyOverwritePrompt);
			} else {
				_destroy(true);
			}
			return;
		}

		// file not found
		if (isSaveDialog) {
			_destroy(true);
		} else {
			Message::createMessageBox("Open",
				"File '" + fileName + "' not found.",
				MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		}
	}

	void FileDialog::notifyOverwritePrompt(Message* sender, MessageBoxStyle result) {
		if (result == MessageBoxStyle::Yes) {
			_destroy(true);
		}
	}

	void FileDialog::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name) {
		if (_name == "close") {
			endMessage();
		}
	}

	void FileDialog::notifyListSelectAccept(MultiListBox* _sender, size_t _position) {
		setFrameAdvise(0); // ???
		if (_position < fileList.size()) {
			txtFileName->setOnlyText(fileList[_position].name);
			cmdOK_Click();
		}
	}

	void FileDialog::notifyListMouseItemActivate(MultiListBox* _sender, size_t _position) {
		if (_position < fileList.size()) {
			txtFileName->setOnlyText(fileList[_position].name);
		}
	}

	void FileDialog::notifyFilterComboAccept(ComboBox* _sender, size_t _index) {
		if (selectedFileType != _index) {
			selectedFileType = _index;
			refreshFileList();
		}
	}

	void FileDialog::_destroy(bool _result) {
		if (_result) {
			eventFileDialogAccept(this);
		}

		delete this;
	}

}
