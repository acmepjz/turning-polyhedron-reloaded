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

		switch (ea.getEventType()) {
		case osgGA::GUIEventAdapter::FRAME:
			level->update();
			return false;
		case osgGA::GUIEventAdapter::KEYDOWN:
			switch (ea.getKey()) {
			case osgGA::GUIEventAdapter::KEY_Up:
				if (!level->isAnimating() && poly) poly->move(level.get(), MOVE_UP);
				break;
			case osgGA::GUIEventAdapter::KEY_Down:
				if (!level->isAnimating() && poly) poly->move(level.get(), MOVE_DOWN);
				break;
			case osgGA::GUIEventAdapter::KEY_Left:
				if (!level->isAnimating() && poly) poly->move(level.get(), MOVE_LEFT);
				break;
			case osgGA::GUIEventAdapter::KEY_Right:
				if (!level->isAnimating() && poly) poly->move(level.get(), MOVE_RIGHT);
				break;
			case osgGA::GUIEventAdapter::KEY_Space:
				if (!level->isAnimating()) level->switchToNextPolyhedron();
				break;
			default:
				return false;
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

	osg::DisplaySettings::instance()->setMinimumNumStencilBits(1);

	//test
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
	node->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace());
	//node->getOrCreateStateSet()->setAttributeAndModes(mat.get());

	osg::ref_ptr<osg::MatrixTransform> mirror = new osg::MatrixTransform(osg::Matrix::scale(1.0f, -1.0f, 1.0f));
	mirror->addChild(node);

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene(/*new osgShadow::ShadowMap*/);
	shadowedScene->addChild(mirror);

	viewer.setSceneData(shadowedScene.get());
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

	viewer.getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	viewer.getCamera()->setClearStencil(0);

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	//viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new TestController(level.get()));

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
