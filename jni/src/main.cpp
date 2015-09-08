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
#include <osgViewer/ViewerEventHandlers>
#include <osgFX/SpecularHighlights>
#include <osgFX/Scribe>

#include "TileType.h"
#include "MapData.h"
#include "SimpleGeometry.h"

using namespace game;
using namespace geom;

osg::Node* test(){
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
		osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			true,
			0.05f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
		geode->addDrawable(ground);
		geode->addDrawable(wireframe);
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
		osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, -0.25f),
			osg::Vec3(1, 1, 0),
			true,
			0.0f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
		geode->addDrawable(ground);
		geode->addDrawable(wireframe);
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
		osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			true,
			0.05f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
		geode->addDrawable(ground);
		geode->addDrawable(wireframe);
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
		osg::ref_ptr<osg::Geometry> wireframe = createCube(
			osg::Vec3(0, 0, 0),
			osg::Vec3(1, 1, 1),
			true,
			0.0f,
			osg::Vec3(0.2f, 0.2f, 0.2f)
			);
		wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
		geode->addDrawable(ground);
		geode->addDrawable(wireframe);
	}
	lod2->addChild(geode, 50.0f, FLT_MAX);

	osg::ref_ptr<osg::Group> gp = new osg::Group;
	gp->addChild(lod);
	gp->addChild(lod2);

	//create some tile types
	osg::ref_ptr<TileType> ground = new TileType;
	ground->id = "ground";
	ground->index = 1;
	ground->name = "Ground";
	ground->desc = "Normal ground.";
	ground->appearance = lod;
	osg::ref_ptr<TileType> wall = new TileType;
	wall->id = "wall";
	wall->index = 11;
	wall->name = "Wall";
	wall->desc = "As an obstacle, your block can't pass through the wall, but...";
	wall->appearance = gp;

	osg::ref_ptr<game::MapData> map = new game::MapData;
	map->resize(osg::Vec3i(), osg::Vec3i(10, 10, 1), false);
	for (int i = 0; i < 100; i++) {
		if (i % 7 == 0) map->tiles[i] = wall.get();
		else if (i % 3 != 0) map->tiles[i] = ground.get();
	}

	//test!!!
	//osgDB::writeObjectFile(*map, "out.osgt");

	return map->createInstance();
}

/*osg::Node* test2(){
	osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile("out.osgt");
	osg::ref_ptr<game::MapData> map = dynamic_cast<game::MapData*>(obj.get());
	return map->createInstance();
}*/

int main(int argc, char** argv){
	osgViewer::Viewer viewer;

	//test
	osg::ref_ptr<osg::Node> node = test();

	//osg::ref_ptr<osg::Material> mat = new osg::Material;
	//mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	//mat->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
	node->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace());
	//node->getOrCreateStateSet()->setAttributeAndModes(mat.get());

	viewer.setSceneData(node.get());
	viewer.setLightingMode(osg::View::SKY_LIGHT);
	viewer.getLight()->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setDiffuse(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.getLight()->setPosition(osg::Vec4(3.0f, 4.0f, 5.0f, 0.0f));

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
