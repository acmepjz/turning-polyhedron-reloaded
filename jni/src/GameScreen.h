#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"

namespace MyGUI {
	class Message;
	struct MessageBoxStyle;
	class FileDialog;
}

#include <osg/Group>
#include <osgGA/OrbitManipulator>
#include "Level.h"
#include "LevelController.h"

class GameScreen : public wraps::BaseLayout {
public:
	GameScreen();

	virtual ~GameScreen();

	/** Set smooth message showing*/
	GameScreen* setSmoothShow(bool _value);

	void endMessage();

	void setLevel(game::Level* level_);

private:
	void notifyMessageBoxResult(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void notifyFileDialogAccept(MyGUI::FileDialog* sender);
	void notifyMenuItemClick(MyGUI::MenuControl* sender, MyGUI::MenuItem* item);
	static void toggleCheck(MyGUI::MenuItem* current);
	void toggleRadio(MyGUI::MenuItem* current);

	void showFileDialog(const std::string& name, const std::string& currentDirectory, const std::string& fileName);
	bool loadFile(const std::string& fullName, const std::string& directory);

	void frameEntered(float _frame);
	void frameAdvise(bool _advise);

private:
	bool mSmoothShow;
	bool mFrameAdvise;

private:
	osg::ref_ptr<game::Level> level;
	osg::ref_ptr<osgGA::OrbitManipulator> cameraController;
	osg::ref_ptr<LevelController> levelController;

private:
	MyGUI::Window* _demoView;
	MyGUI::MenuBar* _menuBar;
	MyGUI::MenuControl *_recentFiles, *_recentFolders;
};
