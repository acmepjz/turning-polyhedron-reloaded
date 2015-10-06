#include "SimpleGeometry.h"

namespace gfx {

	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color)
	{
		osg::Vec3 p[2] = { p1, p2 };
		for (int i = 0; i < 2; i++) {
			if (p[0][i] > p[1][i]) std::swap(p[0][i], p[1][i]);
		}

		osg::ref_ptr<osg::Vec3Array> vv = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec3Array> nn;
		if (!wireframe) nn = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> cc = new osg::Vec3Array(osg::Array::BIND_OVERALL);
		osg::ref_ptr<osg::DrawElementsUInt> ii;

		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		geom->setVertexArray(vv);
		if (!wireframe) geom->setNormalArray(nn);
		geom->setColorArray(cc);

		cc->push_back(color);

		if (bevel < 1E-6f) {
			bevel = 0;
			if (wireframe) {
				for (int i = 0; i < 2; i++)
					for (int j = 0; j < 2; j++)
						for (int k = 0; k < 2; k++)
							vv->push_back(osg::Vec3(p[i][0], p[j][1], p[k][2]));
				const GLuint i1w[] = {
					0, 1, 2, 3, 4, 5, 6, 7,
					0, 2, 1, 3, 4, 6, 5, 7,
					0, 4, 1, 5, 2, 6, 3, 7,
				};
				ii = new osg::DrawElementsUInt(GL_LINES, sizeof(i1w) / sizeof(i1w[0]), i1w);
				geom->addPrimitiveSet(ii);
				return geom.release();
			}
		}

#define ADD_CUBE_FACE(NX,NY,NZ) \
	vv->push_back(osg::Vec3(p[0][0]+(NX?0:bevel), p[0][1]+(NY?0:bevel), p[0][2]+(NZ?0:bevel))); \
	vv->push_back(osg::Vec3(p[NZ][0]+(NX?0:NZ?-bevel:bevel), p[NX][1]+(NY?0:NX?-bevel:bevel), p[NY][2]+(NZ?0:NY?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[NY][0]+(NX?0:NY?-bevel:bevel), p[NZ][1]+(NY?0:NZ?-bevel:bevel), p[NX][2]+(NZ?0:NX?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[1-NX][0]-(NX?0:bevel), p[1-NY][1]-(NY?0:bevel), p[1-NZ][2]-(NZ?0:bevel))); \
	if (!wireframe) nn->insert(nn->end(), 4, osg::Vec3(-NX, -NY, -NZ)); \
	vv->push_back(osg::Vec3(p[1][0]-(NX?0:bevel), p[1][1]-(NY?0:bevel), p[1][2]-(NZ?0:bevel))); \
	vv->push_back(osg::Vec3(p[1-NZ][0]-(NX?0:NZ?-bevel:bevel), p[1-NX][1]-(NY?0:NX?-bevel:bevel), p[1-NY][2]-(NZ?0:NY?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[1-NY][0]-(NX?0:NY?-bevel:bevel), p[1-NZ][1]-(NY?0:NZ?-bevel:bevel), p[1-NX][2]-(NZ?0:NX?-bevel:bevel))); \
	vv->push_back(osg::Vec3(p[NX][0]+(NX?0:bevel), p[NY][1]+(NY?0:bevel), p[NZ][2]+(NZ?0:bevel))); \
	if (!wireframe) nn->insert(nn->end(), 4, osg::Vec3(NX, NY, NZ));

		ADD_CUBE_FACE(1, 0, 0);
		ADD_CUBE_FACE(0, 1, 0);
		ADD_CUBE_FACE(0, 0, 1);

#undef ADD_CUBE_FACE

		const GLuint i1[] = {
#define ADD_RECT(I,J,K,L) I,J,K,J,L,K,I^4,K^4,J^4,J^4,K^4,L^4
#define ADD_RECT2(I) ADD_RECT((I),(I+1),(I+2),(I+3))
#define ADD_TRI(I,J,K) I,J,K,I^4,K^4,J^4
			ADD_RECT2(0), ADD_RECT2(8), ADD_RECT2(16),
			ADD_RECT(0, 2, 8, 9), ADD_RECT(8, 10, 16, 17), ADD_RECT(16, 18, 0, 1),
			ADD_RECT(10, 11, 7, 5), ADD_RECT(18, 19, 15, 13), ADD_RECT(2, 3, 23, 21),
			ADD_TRI(0, 8, 16), ADD_TRI(2, 23, 9), ADD_TRI(10, 7, 17), ADD_TRI(18, 15, 1),
#undef ADD_RECT
#undef ADD_RECT2
#undef ADD_TRI
		};

		const GLuint i1w[] = {
#define ADD_RECT(I) I,I+1,I+1,I+3,I+3,I+2,I+2,I
#define ADD_TRI(I,J,K) I,J,J,K,K,I,I^4,J^4,J^4,K^4,K^4,I^4
			ADD_RECT(0), ADD_RECT(4), ADD_RECT(8),
			ADD_RECT(12), ADD_RECT(16), ADD_RECT(20),
			ADD_TRI(0, 8, 16), ADD_TRI(2, 23, 9), ADD_TRI(10, 7, 17), ADD_TRI(18, 15, 1),
#undef ADD_RECT
#undef ADD_TRI
		};

		if (wireframe) {
			ii = new osg::DrawElementsUInt(GL_LINES, sizeof(i1w) / sizeof(i1w[0]), i1w);
		} else {
			ii = new osg::DrawElementsUInt(GL_TRIANGLES,
				bevel ? (sizeof(i1) / sizeof(i1[0])) : 36, i1);
		}

		geom->addPrimitiveSet(ii);
		return geom.release();
	}

	SimpleGeometry::SimpleGeometry()
	{
	}

	SimpleGeometry::~SimpleGeometry()
	{
	}

	SimpleGeometry::SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop)
		: osg::Object(other, copyop)
	{
	}

}
