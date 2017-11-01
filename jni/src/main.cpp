#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osg/LOD>
#include <osg/Depth>
#include <osg/CullFace>
#include <osgGA/GUIEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgFX/SpecularHighlights>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "TileType.h"
#include "MapData.h"
#include "GameManager.h"
#include "CompressionManager.h"
#include "ConfigManager.h"
#include "LevelController.h"
#include "LevelCollection.h"
#include "util_err.h"

int g_argc = 0;
char **g_argv = NULL;
osgViewer::Viewer *theViewer = NULL;
osg::Group *levelRoot = NULL; // TODO: remove this ad-hoc thing
wraps::BaseLayout *currentScreen = NULL;

#include "MYGUIManager.h"
#include "GameScreen.h"

using namespace game;

class CustomMYGUIManager : public MYGUIManager
{
public:
	CustomMYGUIManager() {}

private:
	virtual void setupResources()
	{
		MYGUIManager::setupResources();
		_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Layout", false);
	}

	virtual void initializeControls()
	{
		currentScreen = new GameScreen();
	}
};

int main(int argc, char** argv){
	g_argc = argc;
	g_argv = argv;

	// set notify level
	//osg::setNotifyLevel(osg::NotifySeverity::WARN);

	//init compression manager
	osg::ref_ptr<CompressionManager> compMgr = new CompressionManager();

	//load config
	osg::ref_ptr<ConfigManager> cfg = new ConfigManager("config.xml");

	//create viewer
	osgViewer::Viewer viewer;
	theViewer = &viewer;

	//create window traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->x = 50;
	traits->y = 50;
	traits->width = 800;
	traits->height = 600;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->stencil = 8;
	traits->windowName = "Turning Polyhedron Reloaded";

	//setup camera
	osg::ref_ptr<osg::Camera> camera = viewer.getCamera();
	camera->setGraphicsContext(osg::GraphicsContext::createGraphicsContext(traits.get()));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));
	camera->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth());

	//load defaults
	osg::ref_ptr<GameManager> gameMgr = new GameManager();
	gameMgr->loadDefaults();

	//osg::ref_ptr<osg::Material> mat = new osg::Material;
	//mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
	//node->getOrCreateStateSet()->setAttributeAndModes(mat.get());

	osg::ref_ptr<osg::MatrixTransform> mirror = new osg::MatrixTransform(osg::Matrix::scale(1.0f, -1.0f, 1.0f));
	levelRoot = mirror.get();

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene(/*new osgShadow::ShadowMap*/);
	shadowedScene->addChild(mirror);
	shadowedScene->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace());

	osg::ref_ptr<osg::Group> root = new osg::Group();
	root->addChild(shadowedScene);

	//test: GUI
	osg::ref_ptr<CustomMYGUIManager> mygui = new CustomMYGUIManager;

	osg::ref_ptr<osg::Geode> ui_geode = new osg::Geode;
	ui_geode->setCullingActive(false);
	ui_geode->addDrawable(mygui.get());
	ui_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	ui_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::ref_ptr<osg::Camera> ui_camera = new osg::Camera;
	ui_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ui_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	ui_camera->setRenderOrder(osg::Camera::POST_RENDER);
	ui_camera->setAllowEventFocus(false);
	ui_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, traits->width, 0.0, traits->height));
	ui_camera->addChild(ui_geode.get());

	root->addChild(ui_camera.get());

	viewer.setSceneData(root.get());
	viewer.setLightingMode(osg::View::SKY_LIGHT);
	viewer.getLight()->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setDiffuse(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setPosition(osg::Vec4(3.0f, 4.0f, 5.0f, 0.0f));

	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded); //otherwise it randomly crashes
	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new MYGUIHandler(ui_camera.get(), mygui.get()));
	//viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	//viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.realize();

	osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(camera->getGraphicsContext());
	if (gw)
	{
		// Send window size for MyGUI to initialize
		int x, y, w, h; gw->getWindowRectangle(x, y, w, h);
		viewer.getEventQueue()->windowResize(x, y, w, h);
		mygui->setGraphicsWindow(gw);
	}

	return viewer.run();
}
