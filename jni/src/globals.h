#pragma once

extern int g_argc;
extern char **g_argv;

namespace osgViewer {
	class Viewer;
}
extern osgViewer::Viewer *theViewer; //!< the global osg viewer

namespace wraps {
	class BaseLayout;
}
extern wraps::BaseLayout *currentScreen; //!< current screen

namespace osg {
	class Group;
}
extern osg::Group *levelRoot; // TODO: remove this ad-hoc thing
