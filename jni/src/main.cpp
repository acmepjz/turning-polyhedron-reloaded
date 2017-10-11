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

#include "TileType.h"
#include "MapData.h"
#include "LevelCollection.h"
#include "SimpleGeometry.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include "MYGUIManager.h"
#include "MessageBox.h"
#include "FileDialog.h"

using namespace game;
using namespace gfx;

class GameManager : public osg::Referenced {
protected:
	virtual ~GameManager() {}
public:
	GameManager() {}

	void loadDefaults();
	game::Level* loadLevel(const char* filename, int levelIndex);
	game::Level* loadOrCreateLevel(const char* filename, int levelIndex);

public:
	osg::ref_ptr<game::ObjectTypeMap> defaultObjectTypeMap;
	osg::ref_ptr<game::TileTypeMap> defaultTileTypeMap;
};

class TestController : public osgGA::GUIEventHandler {
public:
	TestController(game::Level* lv = NULL)
		: level(lv)
	{

	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		if (!level.valid()) return false;
		if (MyGUI::InputManager::getInstancePtr() && MyGUI::InputManager::getInstancePtr()->isModalAny()) return false;

		game::Polyhedron *poly = level->getSelectedPolyhedron();
		int dir = -1;

		switch (ea.getEventType()) {
		case osgGA::GUIEventAdapter::FRAME:
			level->update();
			return false;
		case osgGA::GUIEventAdapter::KEYDOWN:
			switch (ea.getKey()) {
			case osgGA::GUIEventAdapter::KEY_Up:
				dir = 0; break;
			case osgGA::GUIEventAdapter::KEY_Left:
				dir = 1; break;
			case osgGA::GUIEventAdapter::KEY_Down:
				dir = 2; break;
			case osgGA::GUIEventAdapter::KEY_Right:
				dir = 3; break;
			case osgGA::GUIEventAdapter::KEY_Space:
				if (!level->isAnimating()) level->switchToNextPolyhedron();
				break;
			default:
				return false;
			}
			if (dir >= 0) {
				osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
				osg::Camera* camera = viewer ? viewer->getCamera() : NULL;
				if (camera) {
					osg::Vec3 eye, center, up;
					camera->getViewMatrixAsLookAt(eye, center, up);
					eye = (eye - center) ^ up;
					if (eye.x() < eye.y()) {
						if (eye.x() + eye.y() > 0) dir += 3;
					} else if (eye.x() + eye.y() > 0) dir += 2;
					else dir += 1;
				}
				const game::MoveDirection dirs[4] = { MOVE_UP, MOVE_LEFT, MOVE_DOWN, MOVE_RIGHT };
				if (!level->isAnimating() && poly) poly->move(level.get(), dirs[dir & 3]);
			}
			break;
		default:
			return false;
		}
		return true;
	}
public:
	osg::ref_ptr<game::Level> level;
};

// This class is modified from the Demo_Themes example of MyGUI
class CustomMYGUIManager : public MYGUIManager
{
public:
	CustomMYGUIManager()
		: theViewer(NULL)
		, _demoView(NULL)
		, _menuBar(NULL)
	{}

	void setLevel(Level* level_) {
		// reset controller
		if (!levelController.valid()) {
			levelController = new TestController;
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
private:
	virtual void setupResources()
	{
		MYGUIManager::setupResources();
		_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Layout", false);
	}

	virtual void initializeControls()
	{
		MyGUI::VectorWidgetPtr windows = MyGUI::LayoutManager::getInstance().loadLayout("GameScreen.layout");

		_demoView = windows[0]->castType<MyGUI::Window>();
		_menuBar = _demoView->findWidget("MenuBar")->castType<MyGUI::MenuBar>();

		_menuBar->eventMenuCtrlAccept += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMenuItemClick);
	}

	void notifyMessageBoxResult(MyGUI::Message* sender, MyGUI::MessageBoxStyle result) {
		if (sender->mTag == "mnuExit") {
			if (result == MyGUI::MessageBoxStyle::Yes) {
				theViewer->setDone(true);
			}
		}
	}

	void notifyFileDialogAccept(MyGUI::FileDialog* sender) {
		if (sender->mTag == "mnuOpen") {
			Level *newLevel = gameMgr->loadLevel((sender->currentDirectory + sender->fileName).c_str(), 0);
			if (newLevel) setLevel(newLevel);
			else {
				MyGUI::Message::createMessageBox("Error",
					"Failed to load level file '" + sender->fileName + "'.",
					MyGUI::MessageBoxStyle::Ok | MyGUI::MessageBoxStyle::IconWarning);
			}
		}
	}

	void notifyMenuItemClick(MyGUI::MenuControl* sender, MyGUI::MenuItem* item) {
		const std::string& name = item->getName();
		if (name == "mnuExit") {
			MyGUI::Message *msgbox = MyGUI::Message::createMessageBox("Exit game", "Are you sure?",
				MyGUI::MessageBoxStyle::YesNo | MyGUI::MessageBoxStyle::IconWarning);
			msgbox->mTag = name;
			msgbox->eventMessageBoxResult += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMessageBoxResult);
		} else if (name == "mnuOpen" || name == "mnuSaveAs") {
			MyGUI::FileDialog *window = new MyGUI::FileDialog();
			window->isSaveDialog = name == "mnuSaveAs";
			window->addFileType("XML level file", "xml xml.lzma box");
			window->addFileType("All files", "");
			window->setSmoothShow(true);
			window->setMessageModal(true);
			window->mTag = name;
			window->eventFileDialogAccept += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyFileDialogAccept);
			window->initialize();
		} else if (name == "mnuUIScale") {
			toggleRadio(item);
			setUIScale(atof(item->getUserString("Tag").c_str()));
		} else if (name == "mnuMsgBox") {
			MyGUI::Message::createMessageBox("Project1", "Hello, World!");
		}
	}

	static void toggleCheck(MyGUI::MenuItem* current) {
		current->setItemChecked(!current->getItemChecked());
	}

	void toggleRadio(MyGUI::MenuItem* current) {
		MyGUI::VectorWidgetPtr widgets;
		_menuBar->findWidgets(current->getName(), widgets);
		for (size_t i = 0; i < widgets.size(); i++) {
			MyGUI::MenuItem *item = widgets[i]->castType<MyGUI::MenuItem>();
			item->setItemChecked(item == current);
		}
	}

public:
	osgViewer::Viewer *theViewer;
	osg::ref_ptr<GameManager> gameMgr;
	osg::ref_ptr<osg::Group> levelRoot;

private:
	osg::ref_ptr<Level> level;
	osg::ref_ptr<osgGA::OrbitManipulator> cameraController;
	osg::ref_ptr<TestController> levelController;

private:
	MyGUI::Widget* _demoView;
	MyGUI::MenuBar* _menuBar;
};

void GameManager::loadDefaults() {
	defaultObjectTypeMap = new ObjectTypeMap;
	osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultObjectTypes.xml", std::ios::in | std::ios::binary));
	if (x.valid()) defaultObjectTypeMap->load(x.get());
	else UTIL_WARN "Failed to load default object types" << std::endl;

	defaultTileTypeMap = new TileTypeMap;
	x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultTileTypes.xml", std::ios::in | std::ios::binary));
	if (x.valid()) defaultTileTypeMap->load(x.get());
	else UTIL_WARN "Failed to load default tile types" << std::endl;
}

game::Level* GameManager::loadLevel(const char* filename, int levelIndex) {
	if (!filename) return NULL;
	if (filename) {
		osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(std::ifstream(filename, std::ios::in | std::ios::binary));
		if (x.valid()) {
			osg::ref_ptr<osg::Object> obj = LevelCollection::loadLevelOrCollection(x.get(),
				defaultObjectTypeMap, defaultTileTypeMap);
			// check if it is level collection
			{
				LevelCollection *lc = dynamic_cast<LevelCollection*>(obj.get());
				if (lc) {
					if (levelIndex < 0 || levelIndex >= (int)lc->levels.size()) levelIndex = 0;
					osg::ref_ptr<game::Level> level = lc->levels[levelIndex];
					obj = NULL;
					return level.release();
				}
			}
			// check if it is level
			{
				Level *lv = dynamic_cast<Level*>(obj.get());
				if (lv) {
					osg::ref_ptr<game::Level> level = lv;
					obj = NULL;
					return level.release();
				}
			}
		}
	}
	UTIL_ERR "Failed to load level '" << filename << "'" << std::endl;
	return NULL;
}

Level* GameManager::loadOrCreateLevel(const char* filename, int levelIndex) {
	// try to load a level
	Level *ptr = loadLevel(filename, levelIndex);
	if (ptr) return ptr;

	// create a default level
	osg::ref_ptr<game::Level> level = new game::Level;
	level->name = "Unnamed level";
	level->objectTypeMap = new ObjectTypeMap(*defaultObjectTypeMap);
	level->tileTypeMap = new TileTypeMap(*defaultTileTypeMap);

	//some tile types
	osg::ref_ptr<TileType> ground, ground2, wall, ex;
	ground = level->getOrCreateTileTypeMap()->lookup("ground");
	ground2 = level->getOrCreateTileTypeMap()->lookup("block-ground");
	wall = level->getOrCreateTileTypeMap()->lookup("wall");
	ex = level->getOrCreateTileTypeMap()->lookup("goal");

	//create a map
	osg::ref_ptr<game::MapData> dat = new game::MapData;
	dat->id = "m1";
	dat->resize(osg::Vec3i(), osg::Vec3i(10, 6, 3), false);
	{
		const char s[] =
			"11111     "
			"111111    "
			"11111111W "
			" 111111111"
			"    W11811"
			"      111 ";
		for (int i = 0; i < 60; i++) {
			switch (s[i]) {
			case '1':
				dat->tiles[i] = ground;
				break;
			case '8':
				dat->tiles[i] = ex;
				break;
			case 'W':
				dat->tiles[i] = wall;
				break;
			}
		}
		dat->set(4, 0, 2, ground2.get());
	}
	level->addMapData(dat.get());

	//create a polyhedron (test only)
	osg::ref_ptr<game::Polyhedron> poly = new game::Polyhedron;
	poly->id = "p1";
	poly->flags = poly->MAIN | poly->FRAGILE | poly->SUPPORTER | poly->VISIBLE | poly->FLOATING | poly->CONTINUOUS_HITTEST;
	poly->movement = poly->ROLLING_X | poly->MOVING_Y; //test
	poly->controller = poly->PLAYER;
	poly->pos.map = "m1";
	poly->pos.pos.set(1, 1, 0);
#if 1
	poly->pos.flags = poly->pos.ROT_YXZ | poly->pos.UPPER_Y;
	poly->resize(osg::Vec3i(-1, -1, -1), osg::Vec3i(1, 2, 4), true, false); //test
	poly->customShape[3] = 0;
#endif
	level->addPolyhedron(poly.get());

	/*{
		//create a level collection
		osg::ref_ptr<game::LevelCollection> lvs = new game::LevelCollection;
		lvs->name = "Unnamed level pack";
		lvs->levels.push_back(level);

		//test!!!
		osgDB::writeObjectFile(*lvs, "out.osgt");
	}*/

	return level.release();
}

game::Level* test2(){
	osg::ref_ptr<game::Level> level;
	{
		osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile("out.osgt");
		osg::ref_ptr<game::LevelCollection> lvs = dynamic_cast<game::LevelCollection*>(obj.get());
		level = lvs->levels[0];
	}
	return level.release();
}

int main(int argc, char** argv){
	osgViewer::Viewer viewer;

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

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene(/*new osgShadow::ShadowMap*/);
	shadowedScene->addChild(mirror);
	shadowedScene->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace());

	osg::ref_ptr<osg::Group> root = new osg::Group();
	root->addChild(shadowedScene);

	//test: GUI
	osg::ref_ptr<CustomMYGUIManager> mygui = new CustomMYGUIManager;
	mygui->theViewer = &viewer;
	mygui->gameMgr = gameMgr;
	mygui->levelRoot = mirror;

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

	// try to load a level
	int levelIndex = 0;
	if (argc >= 3) {
		sscanf(argv[2], "%d", &levelIndex);
		levelIndex--;
	}
	mygui->setLevel(gameMgr->loadOrCreateLevel(argc >= 2 ? argv[1] : NULL, levelIndex));

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
