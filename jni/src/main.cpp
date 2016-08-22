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

#include "TileType.h"
#include "MapData.h"
#include "LevelCollection.h"
#include "SimpleGeometry.h"
#include "XMLReaderWriter.h"

//======TEST
#include "MYGUIManager.h"

// This class is modified from the Demo_Themes example of MyGUI
class CustomMYGUIManager : public MYGUIManager
{
public:
	CustomMYGUIManager() : _demoView(NULL), _comboSkins(NULL) {}
protected:
	virtual void setupResources()
	{
		MYGUIManager::setupResources();
		_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Demos/Demo_Themes", false);
		_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Demos", false);
		_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Themes", false);
	}

	virtual void initializeControls()
	{
		//MyGUI::LayoutManager::getInstance().loadLayout("Wallpaper.layout");
		const MyGUI::VectorWidgetPtr& root = MyGUI::LayoutManager::getInstance().loadLayout("HelpPanel.layout");
		if (root.size() == 1)
		{
			root.at(0)->findWidget("Text")->castType<MyGUI::TextBox>()->setCaption(
				"Select skin theme in combobox to see default MyGUI themes.");
		}
		createDemo(0);
	}

	void notifyComboAccept(MyGUI::ComboBox* sender, size_t index)
	{
		createDemo(index);
	}

	void createDemo(int index)
	{
		destroyDemo();
		switch (index)
		{
		case 0:
			MyGUI::ResourceManager::getInstance().load("MyGUI_BlueWhiteTheme.xml");
			break;
		case 1:
			MyGUI::ResourceManager::getInstance().load("MyGUI_BlackBlueTheme.xml");
			break;
		case 2:
			MyGUI::ResourceManager::getInstance().load("MyGUI_BlackOrangeTheme.xml");
			break;
		default: break;
		}

		MyGUI::VectorWidgetPtr windows = MyGUI::LayoutManager::getInstance().loadLayout("Themes.layout");
		if (windows.size()<1)
		{
			OSG_WARN << "Error load layout" << std::endl;
			return;
		}

		_demoView = windows[0];
		_comboSkins = MyGUI::Gui::getInstance().findWidget<MyGUI::ComboBox>("Combo");
		if (_comboSkins)
		{
			_comboSkins->setComboModeDrop(true);
			_comboSkins->addItem("blue & white");
			_comboSkins->addItem("black & blue");
			_comboSkins->addItem("black & orange");
			_comboSkins->setIndexSelected(index);
			_comboSkins->eventComboAccept += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyComboAccept);
		}
	}

	void destroyDemo()
	{
		if (_demoView)
			MyGUI::WidgetManager::getInstance().destroyWidget(_demoView);
		_demoView = NULL;
		_comboSkins = NULL;
	}

	MyGUI::Widget* _demoView;
	MyGUI::ComboBox* _comboSkins;
};
//========

using namespace game;
using namespace gfx;

game::Level* test(const char* filename, int levelIndex) {
	//create a level
	osg::ref_ptr<game::Level> level = new game::Level;
	level->name = "Unnamed level";

	//TETS
	osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultObjectTypes.xml", std::ios::in | std::ios::binary));
	if (x.valid()) level->getOrCreateObjectTypeMap()->load(x.get());
	x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultTileTypes.xml", std::ios::in | std::ios::binary));
	if (x.valid()) level->getOrCreateTileTypeMap()->load(x.get());

	//try to load a level
	if (filename) {
		x = XMLReaderWriter::readFile(std::ifstream(filename, std::ios::in | std::ios::binary));
		if (x.valid()) {
			osg::ref_ptr<osg::Object> obj = LevelCollection::loadLevelOrCollection(x.get(),
				level->getOrCreateObjectTypeMap(), level->getOrCreateTileTypeMap());
			//check if it is level collection
			{
				LevelCollection *lc = dynamic_cast<LevelCollection*>(obj.get());
				if (lc) {
					if (levelIndex < 0 || levelIndex >= (int)lc->levels.size()) levelIndex = 0;
					level = lc->levels[levelIndex];
					obj = NULL;
					return level.release();
				}
			}
			//check if it is level
			{
				Level *lv = dynamic_cast<Level*>(obj.get());
				if (lv) {
					level = lv;
					obj = NULL;
					return level.release();
				}
			}
		}
	}

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

	{
		//create a level collection
		osg::ref_ptr<game::LevelCollection> lvs = new game::LevelCollection;
		lvs->name = "Unnamed level pack";
		lvs->levels.push_back(level);

		//test!!!
		osgDB::writeObjectFile(*lvs, "out.osgt");
	}

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

class TestController : public osgGA::GUIEventHandler {
public:
	TestController(game::Level* lv)
		: level(lv)
	{

	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		if (!level.valid()) return false;

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

	//test: create a level
	int levelIndex = 0;
	if (argc >= 3) {
		sscanf(argv[2], "%d", &levelIndex);
		levelIndex--;
	}
	osg::ref_ptr<game::Level> level = test(argc >= 2 ? argv[1] : NULL, levelIndex);
	level->init();
	level->createInstance();
	osg::ref_ptr<osg::Node> node = level->_appearance;

	//osg::ref_ptr<osg::Material> mat = new osg::Material;
	//mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
	//node->getOrCreateStateSet()->setAttributeAndModes(mat.get());

	osg::ref_ptr<osg::MatrixTransform> mirror = new osg::MatrixTransform(osg::Matrix::scale(1.0f, -1.0f, 1.0f));
	mirror->addChild(node);

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

	{
		osgGA::OrbitManipulator *om = new osgGA::OrbitManipulator;
		viewer.setCameraManipulator(om);

		mirror->computeBound();
		osg::BoundingSphere bs = mirror->getBound();
		osg::Vec3 c = bs.center();
		c.z() = 0.0f;
		osg::Vec3 e = c + osg::Vec3(-1, -3, 2) * bs.radius();

		om->setTransformation(e, c, osg::Vec3d(1, 1, 1));
	}

	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded); //otherwise it randomly crashes
	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new MYGUIHandler(ui_camera.get(), mygui.get()));
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	//viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new TestController(level.get()));

	viewer.realize();

	osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(camera->getGraphicsContext());
	if (gw)
	{
		// Send window size for MyGUI to initialize
		int x, y, w, h; gw->getWindowRectangle(x, y, w, h);
		viewer.getEventQueue()->windowResize(x, y, w, h);
	}

	return viewer.run();
}
