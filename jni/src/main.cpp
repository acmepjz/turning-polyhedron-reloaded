#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgUtil/Optimizer>
#include <osgViewer/ViewerEventHandlers>
#include <osgFX/SpecularHighlights>

#include "TileType.h"

using namespace game;

osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, float bevel, const osg::Vec3& color)
{
	osg::Vec3 p[2] = { p1, p2 };
	for (int i = 0; i < 2; i++) {
		if (p[0][i] > p[1][i]) std::swap(p[0][i], p[1][i]);
	}

	osg::ref_ptr<osg::Vec3Array> vv = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec3Array> nn = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec3Array> cc = new osg::Vec3Array(osg::Array::BIND_OVERALL);

	cc->push_back(color);

	if (bevel < 1E-6f) bevel = 0;

#define ADD_CUBE_FACE(NX,NY,NZ) \
	vv->push_back(osg::Vec3(p[0][0]+(NX?0:bevel), p[0][1]+(NY?0:bevel), p[0][2]+(NZ?0:bevel))); \
	vv->push_back(osg::Vec3(p[NZ][0]+(NX?0:NZ?-bevel:bevel), p[NX][1]+(NY?0:NX?-bevel:bevel), p[NY][2]+(NZ?0:NY?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[NY][0]+(NX?0:NY?-bevel:bevel), p[NZ][1]+(NY?0:NZ?-bevel:bevel), p[NX][2]+(NZ?0:NX?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[1-NX][0]-(NX?0:bevel), p[1-NY][1]-(NY?0:bevel), p[1-NZ][2]-(NZ?0:bevel))); \
	nn->insert(nn->end(), 4, osg::Vec3(-NX, -NY, -NZ)); \
	vv->push_back(osg::Vec3(p[1][0]-(NX?0:bevel), p[1][1]-(NY?0:bevel), p[1][2]-(NZ?0:bevel))); \
	vv->push_back(osg::Vec3(p[1-NZ][0]-(NX?0:NZ?-bevel:bevel), p[1-NX][1]-(NY?0:NX?-bevel:bevel), p[1-NY][2]-(NZ?0:NY?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[1-NY][0]-(NX?0:NY?-bevel:bevel), p[1-NZ][1]-(NY?0:NZ?-bevel:bevel), p[1-NX][2]-(NZ?0:NX?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[NX][0]+(NX?0:bevel), p[NY][1]+(NY?0:bevel), p[NZ][2]+(NZ?0:bevel))); \
	nn->insert(nn->end(), 4, osg::Vec3(NX, NY, NZ));

	ADD_CUBE_FACE(1, 0, 0);
	ADD_CUBE_FACE(0, 1, 0);
	ADD_CUBE_FACE(0, 0, 1);

#undef ADD_CUBE_FACE

#define ADD_RECT(I) I,I+1,I+2,I+1,I+3,I+2

	GLuint i1[] = {
		ADD_RECT(0), ADD_RECT(4), ADD_RECT(8),
		ADD_RECT(12), ADD_RECT(16), ADD_RECT(20),
	};

#undef ADD_RECT

	osg::ref_ptr<osg::DrawElementsUInt> ii = new osg::DrawElementsUInt(GL_TRIANGLES,
		sizeof(i1) / sizeof(i1[0]), i1);

	if (bevel) {

#define ADD_RECT(I,J,K,L) I,J,K,J,L,K,I^4,J^4,K^4,J^4,L^4,K^4
#define ADD_TRI(I,J,K) I,J,K,I^4,J^4,K^4

		GLuint i2[] = {
			ADD_RECT(0, 2, 8, 9), ADD_RECT(8, 10, 16, 17), ADD_RECT(16, 18, 0, 1),
			ADD_RECT(10, 11, 7, 5), ADD_RECT(18, 19, 15, 13), ADD_RECT(2, 3, 23, 21),
			ADD_TRI(0, 8, 16), ADD_TRI(2, 23, 9), ADD_TRI(10, 7, 17), ADD_TRI(18, 15, 1),
		};

#undef ADD_RECT
#undef ADD_TRI

		ii->insert(ii->end(), i2, i2 + sizeof(i2) / sizeof(i2[0]));
	}

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setVertexArray(vv);
	geom->setNormalArray(nn);
	geom->setColorArray(cc);
	geom->addPrimitiveSet(ii);
	return geom.release();
}

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
		0.05f,
		osg::Vec3(0.3f, 0.3f, 0.3f)
		));
	ground->appearance = groundNode;

	return NULL;
}

int main(int argc,char** argv){
	osgViewer::Viewer viewer;

	//test
	osg::ref_ptr<osg::Geode> groundNode = new osg::Geode;
	groundNode->addDrawable(createCube(
		osg::Vec3(0, 0, -0.25f),
		osg::Vec3(1, 1, 0),
		0.05f,
		osg::Vec3(0.3f, 0.3f, 0.3f)
		));
	viewer.setSceneData(groundNode.get());
	/*osg::ref_ptr<osgFX::SpecularHighlights> fx = new osgFX::SpecularHighlights;
	fx->addChild(groundNode.get());
	fx->setSpecularColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	fx->setSpecularExponent(50.0f);
	viewer.setSceneData(fx.get());
	*/

	viewer.setRunMaxFrameRate(30.0);
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
