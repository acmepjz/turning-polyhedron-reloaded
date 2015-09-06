#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osg/LOD>
#include <osgUtil/Optimizer>
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
	lod->addChild(geode, 0.0f, 100.0f);
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
	lod->addChild(geode, 100.0f, FLT_MAX);

	//create some tile types
	osg::ref_ptr<TileType> ground = new TileType;
	ground->id = "ground";
	ground->index = 1;
	ground->name = "Ground";
	ground->desc = "Normal ground.";
	ground->appearance = lod;

	osg::ref_ptr<game::MapData> map = new game::MapData;
	map->resize(osg::Vec3i(), osg::Vec3i(10, 10, 1), false);
	for (int i = 0; i < 100; i++) {
		if (i % 3 == 0) map->tiles[i] = ground.get();
	}

	return map->createInstance();
}

int main(int argc,char** argv){
	osgViewer::Viewer viewer;

	//test
	viewer.setSceneData(test());
	viewer.setLightingMode(osg::View::SKY_LIGHT);

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
