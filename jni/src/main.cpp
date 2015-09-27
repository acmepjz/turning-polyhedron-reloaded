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
#include <osgViewer/ViewerEventHandlers>
#include <osgFX/SpecularHighlights>
#include <osgFX/Scribe>

#include "TileType.h"
#include "MapData.h"
#include "LevelCollection.h"
#include "SimpleGeometry.h"

using namespace game;
using namespace geom;

game::Level* test(){
	//create geometry
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	{
		osg::ref_ptr<osg::Geometry> ground = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			false,
			0.05f,
			osg::Vec3(0.4f, 0.4f, 0.4f)
			);
		/*osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			true,
			0.05f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));*/
		geode->addDrawable(ground);
		//geode->addDrawable(wireframe);
	}
	osg::ref_ptr<osg::LOD> lod = new osg::LOD;
	lod->addChild(geode, 0.0f, 50.0f);
	geode = new osg::Geode;
	{
		osg::ref_ptr<osg::Geometry> ground = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			false,
			0.0f,
			osg::Vec3(0.4f, 0.4f, 0.4f)
			);
		/*osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			true,
			0.0f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));*/
		geode->addDrawable(ground);
		//geode->addDrawable(wireframe);
	}
	lod->addChild(geode, 50.0f, FLT_MAX);
	geode = new osg::Geode;
	{
		osg::ref_ptr<osg::Geometry> ground = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			false,
			0.05f,
			osg::Vec3(0.4f, 0.4f, 0.4f)
			);
		/*osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			true,
			0.05f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));*/
		geode->addDrawable(ground);
		//geode->addDrawable(wireframe);
	}
	osg::ref_ptr<osg::LOD> lod2 = new osg::LOD;
	lod2->addChild(geode, 0.0f, 50.0f);
	geode = new osg::Geode;
	{
		osg::ref_ptr<osg::Geometry> ground = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			false,
			0.0f,
			osg::Vec3(0.4f, 0.4f, 0.4f)
			);
		/*osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			true,
			0.0f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));*/
		geode->addDrawable(ground);
		//geode->addDrawable(wireframe);
	}
	lod2->addChild(geode, 50.0f, FLT_MAX);

	osg::ref_ptr<osg::Group> gp = new osg::Group;
	gp->addChild(lod);
	gp->addChild(lod2);

	osg::ref_ptr<osg::Group> gp2 = new osg::Group;
	gp2->addChild(lod);
	geode = new osg::Geode;
	{
		osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			true,
			0.0f,
			osg::Vec3(1.0f, 1.0f, 0.0f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		geode->addDrawable(wireframe);
	}
	gp2->addChild(geode);

	//create some tile types
	const TileType* tt = NULL;
	osg::ref_ptr<TileType> ground = new TileType;
	ground->id = "ground";
	ground->index = 1;
	ground->flags = tt->SUPPORTER | tt->TILT_SUPPORTER;
	ground->name = "Ground";
	ground->desc = "Normal ground.";
	ground->appearance = lod;
	osg::ref_ptr<TileType> wall = new TileType;
	wall->id = "wall";
	wall->index = 11;
	wall->flags = tt->SUPPORTER | tt->TILT_SUPPORTER;
	wall->blockedArea.set(-1, 1);
	wall->name = "Wall";
	wall->desc = "As an obstacle, your block can't pass through the wall, but...";
	wall->appearance = gp;
	osg::ref_ptr<TileType> ex = new TileType;
	ex->id = "goal";
	ex->index = 8;
	ex->flags = tt->SUPPORTER | tt->TILT_SUPPORTER | tt->EXIT;
	ex->name = "Goal";
	ex->desc = "You'll win the game if you get your block to fall into this square hole after visiting all checkpoints.";
	ex->appearance = gp2;

	//create a level
	osg::ref_ptr<game::Level> level = new game::Level;
	level->name = "Unnamed level";
	level->getOrCreateTileTypeMap()->add(ground.get());
	level->getOrCreateTileTypeMap()->add(wall.get());
	level->getOrCreateTileTypeMap()->add(ex.get());

	//create a map
	osg::ref_ptr<game::MapData> dat = new game::MapData;
	dat->id = "m1";
	dat->resize(osg::Vec3i(), osg::Vec3i(10, 6, 1), false);
	{
		const char s[] =
			"111       "
			"111111    "
			"111111111 "
			" 111111111"
			"     11811"
			"      111 ";
		for (int i = 0; i < 60; i++) {
			switch (s[i]) {
			case '1':
				dat->tiles[i] = ground;
				break;
			case '8':
				dat->tiles[i] = ex;
				break;
			}
		}
	}
	level->addMapData(dat.get());

	//create a polyhedron (test only)
	osg::ref_ptr<game::Polyhedron> poly = new game::Polyhedron;
	poly->id = "p1";
	poly->flags = poly->MAIN | poly->FRAGILE | poly->SUPPORTER | poly->VISIBLE;
	poly->movement = poly->ROLLING_ALL;
	poly->controller = poly->PLAYER;
	poly->pos.map = "m1";
	poly->pos.pos.set(1, 1, 0);
	/*poly->pos.flags = poly->pos.ROT_YXZ | poly->pos.UPPER_Y;
	poly->resize(osg::Vec3i(-1, -1, -1), osg::Vec3i(1, 2, 4), true, false); //test
	poly->customShape[3] = 0;*/
	level->addPolyhedron(poly.get());

	//create a level collection
	osg::ref_ptr<game::LevelCollection> lvs = new game::LevelCollection;
	lvs->name = "Unnamed level pack";
	lvs->levels.push_back(level);

	//test!!!
	osgDB::writeObjectFile(*lvs, "out.osgb");
	osgDB::writeObjectFile(*lvs, "out.osgt");
	osgDB::writeObjectFile(*lvs, "out.osgx");

	lvs = NULL;
	return level.release();
}

game::Level* test2(){
	osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile("out.osgb");
	osg::ref_ptr<game::LevelCollection> lvs = dynamic_cast<game::LevelCollection*>(obj.get());
	osg::ref_ptr<game::Level> level = lvs->levels[0];
	lvs = NULL;
	return level.release();
}

class TestController : public osgGA::GUIEventHandler {
public:
	TestController(game::Level* lv)
		: level(lv)
	{

	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		if (!level.valid() || level->polyhedra.empty()) return false;

		game::Polyhedron *poly = level->polyhedra[0];

		switch (ea.getEventType()) {
		case osgGA::GUIEventAdapter::KEYDOWN:
			switch (ea.getKey()) {
			case osgGA::GUIEventAdapter::KEY_Up:
				poly->move(MOVE_UP);
				break;
			case osgGA::GUIEventAdapter::KEY_Down:
				poly->move(MOVE_DOWN);
				break;
			case osgGA::GUIEventAdapter::KEY_Left:
				poly->move(MOVE_LEFT);
				break;
			case osgGA::GUIEventAdapter::KEY_Right:
				poly->move(MOVE_RIGHT);
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

	//test
	osg::ref_ptr<game::Level> level = test();
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

	viewer.setSceneData(mirror.get());
	viewer.setLightingMode(osg::View::SKY_LIGHT);
	viewer.getLight()->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setDiffuse(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setPosition(osg::Vec4(3.0f, 4.0f, 5.0f, 0.0f));

	mirror->computeBound();
	osg::Vec3 c = mirror->getBound().center();
	c.z() = 0.0f;
	osg::Vec3 e = c + osg::Vec3(-1, -3, 2)*0.9f* mirror->getBound().radius();

	viewer.getCamera()->setViewMatrixAsLookAt(e, c, osg::Vec3d(0, 0, 1));
	viewer.getCamera()->setAllowEventFocus(false);

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new TestController(level.get()));

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
