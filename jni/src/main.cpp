#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osgUtil/Optimizer>
#include <osgViewer/ViewerEventHandlers>
#include <osgFX/SpecularHighlights>
#include <osgFX/Scribe>

#include "TileType.h"
#include "SimpleGeometry.h"

using namespace game;
using namespace geom;

osg::Node* test(){
	//create some tile types
	osg::ref_ptr<TileType> ground = new TileType;
	ground->id = "ground";
	ground->index = 1;
	ground->name = "Ground";
	ground->desc = "Normal ground.";

	//create geometry
	osg::ref_ptr<osg::Geode> groundNode = new osg::Geode;
	groundNode->addDrawable(createCube(
		osg::Vec3(0, 0, -0.25f),
		osg::Vec3(1, 1, 0),
		false,
		0.05f,
		osg::Vec3(0.3f, 0.3f, 0.3f)
		));
	ground->appearance = groundNode;

	return NULL;
}

int main(int argc,char** argv){
	osgViewer::Viewer viewer;

	//test
	osg::ref_ptr<osg::Geometry> ground = createCube(
		osg::Vec3(0, 0, -0.25f),
		osg::Vec3(1, 1, 0),
		false,
		0.05f,
		osg::Vec3(0.3f, 0.3f, 0.3f)
		);
	osg::ref_ptr<osg::Geometry> wireframe = createCube(
		osg::Vec3(0, 0, -0.25f),
		osg::Vec3(1, 1, 0),
		true,
		0.05f,
		osg::Vec3(1.0f, 1.0f, 1.0f)
		);
	wireframe->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	ground->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
	//wireframe->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
	//wireframe->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(ground);
	geode->addDrawable(wireframe);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			osg::MatrixTransform *trans = new osg::MatrixTransform;
			trans->setMatrix(osg::Matrix::translate(osg::Vec3(i, j, 0)));
			trans->addChild(geode);
			root->addChild(trans);
		}
	}
	viewer.setSceneData(root.get());
	/*osg::ref_ptr<osgFX::Scribe> fx = new osgFX::Scribe;
	fx->addChild(groundNode.get());
	fx->setWireframeColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	viewer.setSceneData(fx.get());*/

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
