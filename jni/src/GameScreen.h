#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"
#include "MYGUIAccelerator.h"

#include <osg/Group>
#include <osgGA/OrbitManipulator>
#include "Level.h"
#include "LevelController.h"

namespace MyGUI {
	class Message;
	struct MessageBoxStyle;
	class FileDialog;
}

class LevelListScreen;

class GameScreen : public wraps::BaseLayout {
public:
	GameScreen();

	virtual ~GameScreen();

	void endMessage();

	void setLevelOrCollection(osg::Object* level_);
	void restartLevel();

private:
	void notifyMessageBoxResult(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void notifyFileDialogAccept(MyGUI::FileDialog* sender);
	void notifyLevelListAccept(LevelListScreen* sender);
	void notifyMenuItemClick(MyGUI::MenuControl* sender, MyGUI::MenuItem* item);
	void notifyAcceleratorKeyPressed(MYGUIAccelerator* sender, MyGUI::Widget* widget);

	static void toggleCheck(MyGUI::MenuItem* current);
	void toggleRadio(MyGUI::MenuItem* current);

	void showFileDialog(const std::string& name, const std::string& currentDirectory, const std::string& fileName);
	void newFile();
	bool loadFile(const std::string& fullName, const std::string& directory);

	virtual void frameEntered(float _frame) override;

private:
	osg::ref_ptr<osg::Object> levelTemplate; // level or level collection (template)
	osg::ref_ptr<game::Level> level; // the level (copy constructed from template)
	int selectedLevel; // selected level
	osg::ref_ptr<osgGA::OrbitManipulator> cameraController;
	osg::ref_ptr<LevelController> levelController;

	std::string _levelFileName; // level file name with full path

private:
	MyGUI::Window* _demoView;
	MyGUI::MenuBar* _menuBar;
	MyGUI::MenuControl *_recentFiles, *_recentFolders;
	MYGUIAccelerator _accel;
};
