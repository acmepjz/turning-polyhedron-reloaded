#include "GameScreen.h"
#include "MessageBox.h"
#include "FileDialog.h"

#include "globals.h"
#include "Level.h"
#include "GameManager.h"
#include "MYGUIManager.h"

#include <osgViewer/Viewer>

GameScreen::GameScreen() :
	wraps::BaseLayout("GameScreen.layout"),
	mSmoothShow(false),
	_demoView(NULL),
	_menuBar(NULL)
{
	_demoView = dynamic_cast<MyGUI::Window*>(mMainWidget);

	assignWidget(_menuBar, "MenuBar");

	_menuBar->eventMenuCtrlAccept += MyGUI::newDelegate(this, &GameScreen::notifyMenuItemClick);

	// test: try to load a level
	int levelIndex = 0;
	if (g_argc >= 3) {
		sscanf(g_argv[2], "%d", &levelIndex);
		levelIndex--;
	}
	setLevel(gameMgr->loadOrCreateLevel(g_argc >= 2 ? g_argv[1] : NULL, levelIndex));
}

void GameScreen::setLevel(game::Level* level_) {
	// reset controller
	if (!levelController.valid()) {
		levelController = new LevelController;
		theViewer->addEventHandler(levelController.get());
	}
	levelController->level = NULL;

	// init level
	level = level_;
	level->init();
	level->createInstance();
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

void GameScreen::notifyMessageBoxResult(MyGUI::Message* sender, MyGUI::MessageBoxStyle result) {
	if (sender->mTag == "mnuExit") {
		if (result == MyGUI::MessageBoxStyle::Yes) {
			theViewer->setDone(true);
		}
	}
}

void GameScreen::notifyFileDialogAccept(MyGUI::FileDialog* sender) {
	if (sender->mTag == "mnuOpen") {
		game::Level *newLevel = gameMgr->loadLevel((sender->currentDirectory + sender->fileName).c_str(), 0);
		if (newLevel) setLevel(newLevel);
		else {
			MyGUI::Message::createMessageBox("Error",
				"Failed to load level file '" + sender->fileName + "'.",
				MyGUI::MessageBoxStyle::Ok | MyGUI::MessageBoxStyle::IconWarning);
		}
	}
}

void GameScreen::notifyMenuItemClick(MyGUI::MenuControl* sender, MyGUI::MenuItem* item) {
	std::string name = item->getName().substr(mPrefix.size()); // ???

	if (name == "mnuExit") {
		MyGUI::Message *msgbox = MyGUI::Message::createMessageBox("Exit game", "Are you sure?",
			MyGUI::MessageBoxStyle::YesNo | MyGUI::MessageBoxStyle::IconWarning);
		msgbox->mTag = name;
		msgbox->eventMessageBoxResult += MyGUI::newDelegate(this, &GameScreen::notifyMessageBoxResult);
	} else if (name == "mnuOpen" || name == "mnuSaveAs") {
		MyGUI::FileDialog *window = new MyGUI::FileDialog();
		window->isSaveDialog = name == "mnuSaveAs";
		window->addFileType("XML level file", "xml xml.lzma box");
		window->addFileType("All files", "");
		window->setSmoothShow(true);
		window->setMessageModal(true);
		window->mTag = name;
		window->eventFileDialogAccept += MyGUI::newDelegate(this, &GameScreen::notifyFileDialogAccept);
		window->initialize();
	} else if (name == "mnuUIScale") {
		toggleRadio(item);
		myguiMgr->setUIScale(atof(item->getUserString("Tag").c_str()));
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

GameScreen* GameScreen::setSmoothShow(bool _value)
{
	mSmoothShow = _value;
	if (mSmoothShow)
	{
		mMainWidget->setAlpha(MyGUI::ALPHA_MIN);
		mMainWidget->setVisible(true);
		mMainWidget->castType<MyGUI::Window>()->setVisibleSmooth(true);
	}
	return this;
}

void GameScreen::endMessage() {
	delete this;
}
