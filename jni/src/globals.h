#pragma once

extern int g_argc;
extern char **g_argv;

namespace osgViewer {
	class Viewer;
}
extern osgViewer::Viewer *theViewer; //!< the global osg viewer

class GameManager;
extern GameManager* gameMgr; //!< saves some global data used in game (level loading, etc.)

namespace wraps {
	class BaseLayout;
}
extern wraps::BaseLayout *currentScreen; //!< current screen

class MYGUIManager;
extern MYGUIManager* myguiMgr; //!< the node which renders MyGUI

namespace osg {
	class Group;
}
extern osg::Group *levelRoot; // TODO: remove this ad-hoc thing

class ConfigManager;
extern ConfigManager *cfgMgr; //!< the global config manager

class CompressionManager;
extern CompressionManager *compMgr; //!< the global compression (and file) manager
