#include "GameScreen.h"
#include "MessageBox.h"
#include "FileDialog.h"
#include "LevelListScreen.h"
#include "globals.h"
#include "Level.h"
#include "LevelCollection.h"
#include "GameManager.h"
#include "ConfigManager.h"
#include "MYGUIManager.h"
#include "util_err.h"

#include <stdlib.h>
#include <math.h>

#include <osgViewer/Viewer>

enum {
	UPDATE_RECENT_FILES = 0x1,
	UPDATE_RECENT_FOLDERS = 0x2,
	UPDATE_LEVEL_NAME = 0x4,
	UPDATE_LEVEL_STATISTICS = 0x8, // internal
	UPDATE_LEVEL_ALL = UPDATE_LEVEL_NAME | UPDATE_LEVEL_STATISTICS,
};

GameScreen::GameScreen() :
	wraps::BaseLayout("GameScreen.layout"),
	selectedLevel(0)
{
	assignWidget(_menuBar, "MenuBar");

	_menuBar->eventMenuCtrlAccept += MyGUI::newDelegate(this, &GameScreen::notifyMenuItemClick);

	assignWidget(_recentFiles, "RecentFiles");
	assignWidget(_recentFolders, "RecentFolders");

	{
		MyGUI::Widget *temp;
		assignWidget(temp, "Client");
		MYGUIManager::instance->setMousePassthrough(temp);
	}

	ASSIGN_WIDGET0(lblLevelName);
	ASSIGN_WIDGET0(lblMoves);
	ASSIGN_WIDGET0(lblCheckpoints);

	ADDACCEL1("mnuNew", CTRL, N);
	ADDACCEL1("mnuOpen", CTRL, O);
	ADDACCEL1("mnuSave", CTRL, S);
	ADDACCEL1("mnuExit", CTRL, Q);
	ADDACCEL1("mnuRestart", CTRL, R);
	ADDACCEL1("mnuFirstLevel", CTRL, Home);
	ADDACCEL1("mnuPrevLevel", CTRL, Page_Up);
	ADDACCEL1("mnuNextLevel", CTRL, Page_Down);
	ADDACCEL1("mnuLastLevel", CTRL, End);
	ADDACCEL1("mnuLevelList", CTRL, L);

	_accel.eventAcceleratorKeyPressed += MyGUI::newDelegate(this, &GameScreen::notifyAcceleratorKeyPressed);

	setFrameAdvise(-1);

	// test: try to load a level
	int levelIndex = 0;
	if (g_argc >= 3) {
		sscanf(g_argv[2], "%d", &levelIndex);
		levelIndex--;
	}
	setLevelOrCollection(GameManager::instance->loadOrCreateLevel(g_argc >= 2 ? g_argv[1] : NULL, levelIndex));
}

void GameScreen::restartLevel() {
	if (!levelTemplate.valid()) return;

	addFrameAdvise(UPDATE_LEVEL_ALL);

	// reset controller
	if (!levelController.valid()) {
		levelController = new LevelController;
		theViewer->addEventHandler(levelController.get());
	}
	levelController->level = NULL;

	// load level from level collection
	game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
	if (lc) {
		if (selectedLevel < 0 || selectedLevel >= (int)lc->levels.size()) {
			selectedLevel = 0;
		}
		level = new game::Level(*(lc->levels[selectedLevel].get()));
	} else {
		game::Level *lv = dynamic_cast<game::Level*>(levelTemplate.get());
		if (lv) {
			level = new game::Level(*lv);
		} else {
			UTIL_ERR "Invalid level template" << std::endl;
			// create a placeholder level
			level = GameManager::instance->loadOrCreateLevel(NULL, 0);
		}
	}

	// init level
	level->init();
	level->createInstance(false);
	levelController->level = level;

	// add to scene graph
	levelRoot->removeChildren(0, levelRoot->getNumChildren());
	levelRoot->addChild(level->_appearance.get());

	// add camera controller
	if (!cameraController.valid()) cameraController = new osgGA::OrbitManipulator;
	theViewer->setCameraManipulator(cameraController.get());

	levelRoot->computeBound();
	osg::BoundingSphere bs = levelRoot->getBound();
	osg::Vec3 c = bs.center();
	c.z() = 0.0f;
	osg::Vec3 e = c + osg::Vec3(-1, -3, 2) * bs.radius();

	cameraController->setTransformation(e, c, osg::Vec3d(1, 1, 1));
}

void GameScreen::setLevelOrCollection(osg::Object* level_) {
	// init level
	levelTemplate = level_;
	selectedLevel = 0;
	restartLevel();
}

void GameScreen::notifyMessageBoxResult(MyGUI::Message* sender, MyGUI::MessageBoxStyle result) {
	if (sender->mTag == "mnuExit") {
		if (result == MyGUI::MessageBoxStyle::Yes) {
			theViewer->setDone(true);
		}
	}
}

void GameScreen::notifyFileDialogAccept(MyGUI::FileDialog* sender) {
	if (sender->mTag == "mnuOpen") {
		std::string fullName = sender->currentDirectory + sender->fileName;
		loadFile(fullName, sender->currentDirectory);
	}
}

void GameScreen::notifyLevelListAccept(LevelListScreen* sender) {
	selectedLevel = sender->selectedLevel;
	restartLevel();
}

bool GameScreen::loadFile(const std::string& fullName, const std::string& directory) {
	osg::Object *newLevel = GameManager::instance->loadLevelOrCollection(fullName.c_str());
	if (newLevel) {
		if (ConfigManager::instance->recentFiles.add(fullName)) addFrameAdvise(UPDATE_RECENT_FILES);
		if (!directory.empty() && ConfigManager::instance->recentFolders.add(directory)) addFrameAdvise(UPDATE_RECENT_FOLDERS);

		_levelFileName = fullName;
		setLevelOrCollection(newLevel);

		return true;
	} else {
		if (ConfigManager::instance->recentFiles.remove(fullName)) addFrameAdvise(UPDATE_RECENT_FILES);

		MyGUI::Message::createMessageBox("Error",
			"Failed to load level file '" + fullName + "'.",
			MyGUI::MessageBoxStyle::Ok | MyGUI::MessageBoxStyle::IconWarning);

		return false;
	}
}

void GameScreen::newFile() {
	game::Level *newLevel = GameManager::instance->loadLevel(NULL, 0);
	_levelFileName.clear();
	setLevelOrCollection(newLevel);
}

void GameScreen::frameEntered(float _frame) {
	int fa = getFrameAdvise();

	if (fa & UPDATE_RECENT_FILES) ConfigManager::instance->recentFiles.updateMenu(_recentFiles, "mnuRecentFile");
	if (fa & UPDATE_RECENT_FOLDERS) ConfigManager::instance->recentFolders.updateMenu(_recentFolders, "mnuRecentFolder");

	if (fa & UPDATE_LEVEL_NAME) {
		std::string s;
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
		if (lc) {
			char c[32];
			itoa(selectedLevel + 1, c, 10);
			s = "Level " + std::string(c) + ": " + level->name;
		} else {
			s = level->name;
		}
		lblLevelName->setCaption(s);
	}

	if (fa & UPDATE_LEVEL_ALL) {
		// update moves
		const int m = level->_moves;
		if ((fa & UPDATE_LEVEL_NAME) || m != _tempMoves) {
			_tempMoves = m;

			std::string s;
			char c[32];
			itoa(m, c, 10);
			s = "Moves: " + std::string(c);
			lblMoves->setCaption(s);
		}

		// update checkpoints
		const int r0 = level->getCheckpointRequired();
		const int r1 = r0 > 0 ? level->_checkpointObtained : 0;

		if ((fa & UPDATE_LEVEL_NAME) || r0 != _tempCheck0 || r1 != _tempCheck1) {
			_tempCheck0 = r0;
			_tempCheck1 = r1;

			if (r0 > 0) {
				std::string s;
				char c[32];
				itoa(r1, c, 10);
				s = "Checkpoints: " + std::string(c) + "/";
				itoa(r0, c, 10);
				s += c;
				lblCheckpoints->setCaption(s);
			} else {
				lblCheckpoints->setCaption("");
			}
		}
	}

	setFrameAdvise(UPDATE_LEVEL_STATISTICS);
}

void GameScreen::showFileDialog(const std::string& name, const std::string& currentDirectory, const std::string& fileName) {
	MyGUI::FileDialog *window = new MyGUI::FileDialog();
	window->isSaveDialog = name == "mnuSaveAs";
	window->currentDirectory = currentDirectory;
	window->fileName = fileName;
	window->addFileType("XML level file", "xml xml.lzma xml.xz");
	window->addFileType("TurningSquare level file", "box box.lzma box.xz");
	window->addFileType("PuzzleBoy level file", "lev lev.lzma lev.xz");
	window->addFileType("All files", "");
	window->smoothShow();
	window->setModal(true);
	window->mTag = name;
	window->eventFileDialogAccept += MyGUI::newDelegate(this, &GameScreen::notifyFileDialogAccept);
	window->initialize();
}

void GameScreen::notifyAcceleratorKeyPressed(MYGUIAccelerator* sender, MyGUI::Widget* widget) {
	MyGUI::MenuItem* item = widget->castType<MyGUI::MenuItem>(false);
	if (item) notifyMenuItemClick(NULL, item);
}

void GameScreen::notifyMenuItemClick(MyGUI::MenuControl* sender, MyGUI::MenuItem* item) {
	std::string name = removePrefix(item->getName());

	if (name == "mnuExit") {
		MyGUI::Message *msgbox = MyGUI::Message::createMessageBox("Exit game", "Are you sure?",
			MyGUI::MessageBoxStyle::YesNo | MyGUI::MessageBoxStyle::IconWarning);
		msgbox->mTag = name;
		msgbox->eventMessageBoxResult += MyGUI::newDelegate(this, &GameScreen::notifyMessageBoxResult);
	} else if (name == "mnuNew") {
		newFile();
	} else if (name == "mnuOpen") {
		showFileDialog(name, "", "");
	} else if (name == "mnuSaveAs") {
		if (_levelFileName.empty()) {
			showFileDialog(name, "", "");
		} else {
			size_t lp = _levelFileName.find_last_of("\\/");
			showFileDialog(name,
				lp == std::string::npos ? std::string() : _levelFileName.substr(0, lp + 1),
				lp == std::string::npos ? _levelFileName : _levelFileName.substr(lp + 1));
		}
	} else if (name == "mnuRecentFile") {
		loadFile(item->getUserString("Tag"), "");
	} else if (name == "mnuRecentFolder") {
		showFileDialog("mnuOpen", item->getUserString("Tag"), "");
	} else if (name == "mnuRestart") {
		restartLevel();
	} else if (name == "mnuFirstLevel") {
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
		if (lc && selectedLevel > 0) {
			selectedLevel = 0;
			restartLevel();
		}
	} else if (name == "mnuPrevLevel") {
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
		if (lc && selectedLevel > 0) {
			selectedLevel--;
			restartLevel();
		}
	} else if (name == "mnuNextLevel") {
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
		if (lc && selectedLevel < (int)lc->levels.size() - 1) {
			selectedLevel++;
			restartLevel();
		}
	} else if (name == "mnuLastLevel") {
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelTemplate.get());
		if (lc && selectedLevel < (int)lc->levels.size() - 1) {
			selectedLevel = lc->levels.size() - 1;
			restartLevel();
		}
	} else if (name == "mnuLevelList") {
		LevelListScreen *window = new LevelListScreen();
		window->levelOrLevelCollection = levelTemplate;
		window->selectedLevel = selectedLevel;
		window->smoothShow();
		window->setModal(true);
		window->eventLevelListScreenAccept += MyGUI::newDelegate(this, &GameScreen::notifyLevelListAccept);
		window->initialize();
	} else if (name == "mnuUIScale") {
		toggleRadio(item);
		MYGUIManager::instance->setUIScale(atof(item->getUserString("Tag").c_str()));
	} else if (name == "mnuLogLevel") {
		toggleRadio(item);
		const std::string& tag = item->getUserString("Tag");
		if (tag == "Debug") {
			MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Info);
			osg::setNotifyLevel(osg::NotifySeverity::DEBUG_INFO);
		} else if (tag == "Info") {
			MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Info);
			osg::setNotifyLevel(osg::NotifySeverity::INFO);
		} else if (tag == "Notice") {
			MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Warning);
			osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
		} else if (tag == "Warning") {
			MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Warning);
			osg::setNotifyLevel(osg::NotifySeverity::WARN);
		} else if (tag == "Error") {
			MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Error);
			osg::setNotifyLevel(osg::NotifySeverity::FATAL);
		}
	} else if (name == "mnuMsgBox") {
		MyGUI::Message::createMessageBox("Project1", "Hello, World!");
	}
}

void GameScreen::toggleCheck(MyGUI::MenuItem* current) {
	current->setItemChecked(!current->getItemChecked());
}

void GameScreen::toggleRadio(MyGUI::MenuItem* current) {
	MyGUI::VectorWidgetPtr widgets;
	_menuBar->findWidgets(current->getName(), widgets);
	for (size_t i = 0; i < widgets.size(); i++) {
		MyGUI::MenuItem *item = widgets[i]->castType<MyGUI::MenuItem>();
		item->setItemChecked(item == current);
	}
}

GameScreen::~GameScreen() {
}

void GameScreen::endMessage() {
	delete this;
}
