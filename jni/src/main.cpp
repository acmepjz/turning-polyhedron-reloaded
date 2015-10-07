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

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>

#include "TileType.h"
#include "MapData.h"
#include "LevelCollection.h"
#include "SimpleGeometry.h"
#include "XMLReaderWriter.h"

using namespace game;
using namespace gfx;

game::Level* test(){
	//create geometry
	osg::ref_ptr<gfx::Appearance> ground_a = new gfx::Appearance;
	ground_a->type = gfx::Appearance::MESH_CUBE;
	ground_a->scale.set(1, 1, 0.25f);
	ground_a->center.set(0, 0, 1);
	ground_a->solidColor.set(0.4f, 0.4f, 0.4f);
	ground_a->bevel = 0.05f;
	ground_a->lod = true;

	osg::ref_ptr<gfx::Appearance> wall_a2 = new gfx::Appearance;
	wall_a2->type = gfx::Appearance::MESH_CUBE;
	wall_a2->scale.set(1, 1, 1);
	wall_a2->solidColor.set(0.4f, 0.4f, 0.4f);
	wall_a2->bevel = 0.05f;
	wall_a2->lod = true;

	osg::ref_ptr<gfx::Appearance> wall_a = new gfx::Appearance;
	wall_a->subNodes.push_back(ground_a);
	wall_a->subNodes.push_back(wall_a2);

	osg::ref_ptr<gfx::Appearance> ground2_a = new gfx::Appearance;
	ground2_a->type = gfx::Appearance::TRANSFORM;
	ground2_a->pos.set(0, 0, -1);
	ground2_a->subNodes.push_back(wall_a2);

	osg::ref_ptr<gfx::Appearance> ex_a2 = new gfx::Appearance;
	ex_a2->type = gfx::Appearance::MESH_CUBE;
	ex_a2->scale.set(1, 1, 1);
	ex_a2->solid = false;
	ex_a2->wireframe = true;
	ex_a2->wireframeColor.set(1, 1, 0);

	osg::ref_ptr<gfx::Appearance> ex_a = new gfx::Appearance;
	ex_a->subNodes.push_back(ground_a);
	ex_a->subNodes.push_back(ex_a2);

	//create some tile types
	osg::ref_ptr<TileType> ground = new TileType;
	ground->id = "ground";
	ground->index = 1;
	ground->flags = TileType::SUPPORTER | TileType::TILT_SUPPORTER;
	ground->name = "Ground";
	ground->desc = "Normal ground.";
	ground->appearance = ground_a;

	osg::ref_ptr<TileType> ground2 = new TileType;
	ground2->id = "block-ground";
	ground2->flags = TileType::SUPPORTER | TileType::TILT_SUPPORTER;
	ground2->appearance = ground2_a;

	osg::ref_ptr<TileType> wall = new TileType;
	wall->id = "wall";
	wall->index = 11;
	wall->flags = TileType::SUPPORTER | TileType::TILT_SUPPORTER;
	wall->blockedArea.set(-1, 1);
	wall->name = "Wall";
	wall->desc = "As an obstacle, your block can't pass through the wall, but...";
	wall->appearance = wall_a;

	osg::ref_ptr<TileType> ex = new TileType;
	ex->id = "goal";
	ex->index = 8;
	ex->flags = TileType::SUPPORTER | TileType::TILT_SUPPORTER | TileType::EXIT;
	ex->name = "Goal";
	ex->desc = "You'll win the game if you get your block to fall into this square hole after visiting all checkpoints.";
	ex->appearance = ex_a;

	//create a level
	osg::ref_ptr<game::Level> level = new game::Level;
	level->name = "Unnamed level";
	level->getOrCreateTileTypeMap()->add(ground.get());
	level->getOrCreateTileTypeMap()->add(wall.get());
	level->getOrCreateTileTypeMap()->add(ex.get());

	//TETS
	osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(
		std::ifstream("../../turningpolyhedron/turningpolyhedron/data/DefaultObjectTypes.xml", std::ios::in | std::ios::binary));
	if (x.valid()) level->getOrCreateObjectTypeMap()->load(x.get());

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
		dat->set(3, 0, 2, ground2.get());
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

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene(/*new osgShadow::ShadowMap*/);
	shadowedScene->addChild(mirror);

	viewer.setSceneData(shadowedScene.get());
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
