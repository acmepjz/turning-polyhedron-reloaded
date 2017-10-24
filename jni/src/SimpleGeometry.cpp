#include "SimpleGeometry.h"
#include "XMLReaderWriter.h"
#include "util_err.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace gfx {

#if 0
	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color) {
#if 1
		// test only
		osg::ref_ptr<SimpleGeometry> geom = new SimpleGeometry;
		geom->addCube(p1, p2, bevel);
		return wireframe ? geom->createEdges(color) : geom->createFaces(color, false, true);
#else
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
#endif
	}
#endif

	void SimpleGeometry::addCube(const osg::Vec3& p1, const osg::Vec3& p2, float bevel) {
		osg::Vec3 p[2] = { p1, p2 };
		for (int i = 0; i < 3; i++) {
			if (p[0][i] > p[1][i]) std::swap(p[0][i], p[1][i]);
		}

		std::vector<osg::Vec3> vv;

		const int i2[] = {
			4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4,
			3, 3, 3, 3, 3, 3, 3, 3
		};

		if (bevel < 1E-6f) {
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 2; j++)
					for (int k = 0; k < 2; k++)
						vv.push_back(osg::Vec3(p[i][0], p[j][1], p[k][2]));

#define ADD_RECT(I,J,K,L) I,J,L,K,I^7,K^7,L^7,J^7

			const int i1[] = {
				ADD_RECT(0, 2, 1, 3), ADD_RECT(0, 4, 2, 6), ADD_RECT(0, 1, 4, 5),
			};

#undef ADD_RECT

			addPolyhedron(&(vv[0]), vv.size(), i1, i2, 6);
		} else {

#define ADD_CUBE_FACE(NX,NY,NZ) \
	vv.push_back(osg::Vec3(p[0][0]+(NX?0:bevel), p[0][1]+(NY?0:bevel), p[0][2]+(NZ?0:bevel))); \
	vv.push_back(osg::Vec3(p[NZ][0]+(NX?0:NZ?-bevel:bevel), p[NX][1]+(NY?0:NX?-bevel:bevel), p[NY][2]+(NZ?0:NY?-bevel:bevel))); \
	vv.push_back(osg::Vec3(p[NY][0]+(NX?0:NY?-bevel:bevel), p[NZ][1]+(NY?0:NZ?-bevel:bevel), p[NX][2]+(NZ?0:NX?-bevel:bevel))); \
	vv.push_back(osg::Vec3(p[1-NX][0]-(NX?0:bevel), p[1-NY][1]-(NY?0:bevel), p[1-NZ][2]-(NZ?0:bevel))); \
	vv.push_back(osg::Vec3(p[1][0]-(NX?0:bevel), p[1][1]-(NY?0:bevel), p[1][2]-(NZ?0:bevel))); \
	vv.push_back(osg::Vec3(p[1-NZ][0]-(NX?0:NZ?-bevel:bevel), p[1-NX][1]-(NY?0:NX?-bevel:bevel), p[1-NY][2]-(NZ?0:NY?-bevel:bevel))); \
	vv.push_back(osg::Vec3(p[1-NY][0]-(NX?0:NY?-bevel:bevel), p[1-NZ][1]-(NY?0:NZ?-bevel:bevel), p[1-NX][2]-(NZ?0:NX?-bevel:bevel))); \
	vv.push_back(osg::Vec3(p[NX][0]+(NX?0:bevel), p[NY][1]+(NY?0:bevel), p[NZ][2]+(NZ?0:bevel))); \

			ADD_CUBE_FACE(1, 0, 0);
			ADD_CUBE_FACE(0, 1, 0);
			ADD_CUBE_FACE(0, 0, 1);

#undef ADD_CUBE_FACE

#define ADD_RECT(I,J,K,L) I,J,L,K,I^4,K^4,L^4,J^4
#define ADD_RECT2(I) ADD_RECT((I),(I+1),(I+2),(I+3))
#define ADD_TRI(I,J,K) I,J,K,I^4,K^4,J^4

			const int i1[] = {
				ADD_RECT2(0), ADD_RECT2(8), ADD_RECT2(16),
				ADD_RECT(0, 2, 8, 9), ADD_RECT(8, 10, 16, 17), ADD_RECT(16, 18, 0, 1),
				ADD_RECT(10, 11, 7, 5), ADD_RECT(18, 19, 15, 13), ADD_RECT(2, 3, 23, 21),
				ADD_TRI(0, 8, 16), ADD_TRI(2, 23, 9), ADD_TRI(10, 7, 17), ADD_TRI(18, 15, 1),
			};

			addPolyhedron(&(vv[0]), vv.size(), i1, i2, sizeof(i2) / sizeof(i2[0]));
		}

#undef ADD_RECT
#undef ADD_RECT2
#undef ADD_TRI

	}

	struct SimpleGeometry::Vertex {
		Vertex() : edge(NULL), tempIndex(0) {}

		osg::Vec3 pos; //!< position
		Halfedge *edge; //!< an outgoing halfedge

		int tempIndex;
	};

	struct SimpleGeometry::Halfedge {
		Halfedge() : vertex(NULL), face(NULL), next(NULL), opposite(NULL), temp(0) {}

		Vertex *vertex; //!< the vertex it points to
		Face *face; //!< the face it belongs to
		Halfedge *next; //!< the next halfedge inside the face
		Halfedge *opposite; //!< the opposite halfedge

		int temp;
	};

	struct SimpleGeometry::Face {
		Face() : edge(NULL) {}

		Halfedge *edge; //!< one of the halfedges bounding it

		osg::ref_ptr<Triangulation> triangulation; //!< the triangulation (optional)

		/// flip the face direction
		void flip() {
			Halfedge *oldEdge = edge, *e0 = edge;
			Halfedge *e1 = edge = edge->next;

			Vertex *v0 = e0->vertex;

			for (;;) {
				std::swap(e1->vertex, v0);

				Halfedge *tmp = e1->next;
				e1->next = e0;
				if (e1 == oldEdge) break;
				e0 = e1;
				e1 = tmp;
			}

			if (triangulation.valid()) (int&)(triangulation->type) ^= Triangulation::FLIPPED;
		}

		void calculate(int* vertexCount, osg::Vec3* faceNormal, osg::Vec3* faceCenter, std::vector<Halfedge*>* halfedges) const {
			Halfedge* e0 = edge;
			if (e0 == NULL) {
				if (vertexCount) *vertexCount = 0;
				if (faceNormal) *faceNormal = osg::Vec3(0, 0, 1);
				if (faceCenter) *faceCenter = osg::Vec3();
				return;
			}

			if (faceCenter) *faceCenter = e0->vertex->pos;
			if (halfedges) halfedges->push_back(e0);

			osg::Vec3 prevVector;
			int m = 1;

			for (Halfedge* e = e0->next; e != e0; e = e->next) {
				if (halfedges) halfedges->push_back(e);
				m++;

				if (faceNormal) {
					osg::Vec3 currentVector = e->vertex->pos - e0->vertex->pos;
					if (m >= 3) *faceNormal += prevVector ^ currentVector;
					prevVector = currentVector;
				}
				if (faceCenter) *faceCenter += e->vertex->pos;
			}

			if (vertexCount) *vertexCount = m;
			if (faceCenter) *faceCenter /= m;
		}
	};

	Triangulation::Triangulation()
		: type(TRIANGLE_FAN)
	{
	}

	Triangulation::~Triangulation()
	{
	}

	Triangulation::Triangulation(const Triangulation& other, const osg::CopyOp& copyop)
		: osg::Object(other, copyop)
		, type(other.type)
		, indices(other.indices)
	{
	}

	void Triangulation::addTriangulation(const std::vector<SimpleGeometry::Halfedge*>& es, osg::DrawElementsUInt* ii, int centerIndex) const {
		if (centerIndex >= 0) {
			// forced to use triangle fan mode starting with center
			const int m = es.size();
			int i1 = es[m - 1]->vertex->tempIndex;
			for (int i = 0; i < m; i++) {
				int i2 = es[i]->vertex->tempIndex;
				ii->push_back(centerIndex); ii->push_back(i1); ii->push_back(i2);
				i1 = i2;
			}
			return;
		}

		if (this) {
			switch (type & TYPE_MASK) {
			case TRIANGLES:
				if (indices.size() >= 3) {
					for (int i = 0, m = indices.size(); i + 3 <= m; i += 3) {
						ii->push_back(es[flipIfNecessary(indices[i], es.size())]->vertex->tempIndex);
						if (isFlipped()) {
							ii->push_back(es[flip(indices[i + 2], es.size())]->vertex->tempIndex);
							ii->push_back(es[flip(indices[i + 1], es.size())]->vertex->tempIndex);
						} else {
							ii->push_back(es[indices[i + 1]]->vertex->tempIndex);
							ii->push_back(es[indices[i + 2]]->vertex->tempIndex);
						}
					}
					return;
				}
				break;
			case TRIANGLE_STRIP:
				if (indices.size() >= 3) {
					int i0 = es[flipIfNecessary(indices[0], es.size())]->vertex->tempIndex;
					int i1 = es[flipIfNecessary(indices[1], es.size())]->vertex->tempIndex;
					for (int i = 2, m = indices.size(); i < m; i++) {
						if (((i & 1) != 0) ^ isFlipped()) {
							ii->push_back(i1); ii->push_back(i0);
						} else {
							ii->push_back(i0); ii->push_back(i1);
						}
						int i2 = es[flipIfNecessary(indices[i], es.size())]->vertex->tempIndex;
						ii->push_back(i2);
						i0 = i1; i1 = i2;
					}
					return;
				} else if (indices.size() >= 1) {
					const int m = es.size();
					int idxStart = indices[0];
					if (idxStart < 0 || idxStart >= m) idxStart = 0;
					if (isFlipped()) idxStart = m - 1 - idxStart; //FIXME: this is not accurate, only works if all quads are convex
					int i0 = es[idxStart]->vertex->tempIndex;
					int i1 = es[(idxStart + 1 >= m) ? 0 : (idxStart + 1)]->vertex->tempIndex;
					for (int i = 2; i < m; i++) {
						int idx;
						if (i & 1) {
							ii->push_back(i1); ii->push_back(i0);
							idx = idxStart + (i >> 1) + 1;
							if (idx >= m) idx -= m;
						} else {
							ii->push_back(i0); ii->push_back(i1);
							idx = idxStart - (i >> 1);
							if (idx < 0) idx += m;
						}
						int i2 = es[idx]->vertex->tempIndex;
						ii->push_back(i2);
						i0 = i1; i1 = i2;
					}
					return;
				}
				break;
			case TRIANGLE_FAN:
				if (indices.size() >= 3) {
					int i0 = es[flipIfNecessary(indices[0], es.size())]->vertex->tempIndex;
					int i1 = es[flipIfNecessary(indices[1], es.size())]->vertex->tempIndex;
					for (int i = 2, m = indices.size(); i < m; i++) {
						int i2 = es[flipIfNecessary(indices[i], es.size())]->vertex->tempIndex;
						if (isFlipped()) {
							ii->push_back(i1); ii->push_back(i0);
						} else {
							ii->push_back(i0); ii->push_back(i1);
						}
						ii->push_back(i2);
						i1 = i2;
					}
					return;
				} else if (indices.size() >= 1) {
					const int m = es.size();
					int idxStart = indices[0];
					if (isFlipped()) idxStart = m - idxStart;
					if (idxStart < 0 || idxStart >= m) idxStart = 0;
					int i0 = es[idxStart]->vertex->tempIndex;
					int i1 = es[(idxStart + 1 >= m) ? 0 : (idxStart + 1)]->vertex->tempIndex;
					for (int i = 2; i < m; i++) {
						int idx = idxStart + i;
						if (idx >= m) idx -= m;
						int i2 = es[idx]->vertex->tempIndex;
						ii->push_back(i0); ii->push_back(i1); ii->push_back(i2);
						i1 = i2;
					}
					return;
				}
				break;
			case QUADS:
				if (indices.size() >= 4) {
					for (int i = 0, m = indices.size(); i + 4 <= m; i += 4) {
						int i0 = es[flipIfNecessary(indices[i], es.size())]->vertex->tempIndex,
							i2 = es[flipIfNecessary(indices[i + 2], es.size())]->vertex->tempIndex;
						ii->push_back(i0);
						if (isFlipped()) {
							ii->push_back(i2); ii->push_back(es[flip(indices[i + 1], es.size())]->vertex->tempIndex);
							ii->push_back(i0); ii->push_back(es[flip(indices[i + 3], es.size())]->vertex->tempIndex);
						} else {
							ii->push_back(es[indices[i + 1]]->vertex->tempIndex); ii->push_back(i2);
							ii->push_back(es[indices[i + 3]]->vertex->tempIndex); ii->push_back(i0);
						}
						ii->push_back(i2);
					}
					return;
				}
				break;
			}
		}

		// default mode is triangle fan
		int i0 = es[0]->vertex->tempIndex;
		int i1 = es[1]->vertex->tempIndex;
		for (int i = 2, m = es.size(); i < m; i++) {
			int i2 = es[i]->vertex->tempIndex;
			ii->push_back(i0); ii->push_back(i1); ii->push_back(i2);
			i1 = i2;
		}
	}

	bool Triangulation::load(const XMLNode* node) {
		std::string s = node->getAttr("type", std::string());
		if (s.empty() || s == "triangleFan") type = TRIANGLE_FAN;
		else if (s == "triangles") type = TRIANGLES;
		else if (s == "triangleStrip") type = TRIANGLE_STRIP;
		else if (s == "quads") type = QUADS;
		else {
			UTIL_WARN "unrecognized triangulation type: " << s << std::endl;
			return false;
		}

		if (node->getAttr("flipped", false)) (int&)type |= FLIPPED;

		if (!node->contents.empty()) {
			const char* s = node->contents.c_str();
			for (;;) {
				int i = 0;
				if (sscanf(s, "%d", &i) != 1) break;
				indices.push_back(i);
				s = strchr(s, ',');
				if (s == NULL) break;
				s++;
				if (*s == 0) break;
			}
		}

		return true;
	}

	bool Triangulation::valid(int vertexCount) const {
		for (size_t i = 0, m = indices.size(); i < m; i++) {
			int idx = indices[i];
			if (idx < 0 || idx >= vertexCount) return false;
		}
		return true;
	}

	SimpleGeometry::SimpleGeometry()
	{
	}

	SimpleGeometry::~SimpleGeometry()
	{
		clear();
	}

	//TODO: copy constructor
	SimpleGeometry::SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop)
		: osg::Object(other, copyop)
	{
	}

	void SimpleGeometry::clear() {
		for (size_t i = 0, m = vertices.size(); i < m; i++) {
			delete vertices[i];
		}
		for (size_t i = 0, m = halfedges.size(); i < m; i++) {
			delete halfedges[i];
		}
		for (size_t i = 0, m = faces.size(); i < m; i++) {
			delete faces[i];
		}
		vertices.clear();
		halfedges.clear();
		faces.clear();
	}

	SimpleGeometry::Face* SimpleGeometry::addPolygon(const osg::Vec3* vertices_, int vertexCount, Triangulation* triangulation) {
		std::vector<Vertex*> newVertices;

		// create new vertices
		for (int i = 0; i < vertexCount; i++) {
			Vertex *v = new Vertex;
			v->pos = vertices_[i];
			newVertices.push_back(v);
		}

		// create new face
		Face *f = new Face;
		f->triangulation = triangulation;

		int prevIndex = vertexCount - 1;
		Halfedge *prevEdge = NULL;

		for (int i = 0; i < vertexCount; i++) {
			Halfedge *e = new Halfedge;

			if (i == 0) {
				// init first edge
				f->edge = e;
			} else {
				// set next edge of prev edge
				prevEdge->next = e;
			}

			// set end vertex
			e->vertex = newVertices[i];

			// set start vertex
			Vertex *prevVertex = newVertices[prevIndex];
			if (prevVertex->edge == NULL) prevVertex->edge = e;

			// set face
			e->face = f;

			// over
			halfedges.push_back(e);
			prevIndex = i;
			prevEdge = e;
		}

		// set next edge of last edge
		prevEdge->next = f->edge;

		// add new face
		faces.push_back(f);

		// add to existing data
		vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());

		return f;
	}

	SimpleGeometry::Face* SimpleGeometry::addRect(const osg::Vec3& p1, const osg::Vec3& p2, float bevel, int segments) {
		float p[2][2] = { p1.x(), p1.y(), p2.x(), p2.y() };
		for (int i = 0; i < 2; i++) {
			if (p[0][i] > p[1][i]) std::swap(p[0][i], p[1][i]);
		}
		float q[4][6] = {
			p[0][0], p[0][1], 0, bevel, bevel, 0,
			p[1][0], p[0][1], -bevel, 0, 0, bevel,
			p[1][0], p[1][1], 0, -bevel, -bevel, 0,
			p[0][0], p[1][1], bevel, 0, 0, -bevel,
		};

		std::vector<osg::Vec3> vv;

		if (bevel < 1E-6f) segments = 0;
		else if (segments <= 0) segments = 1;

		for (int i = 0; i < 4; i++) {
			if (segments <= 0) {
				vv.push_back(osg::Vec3(q[i][0], q[i][1], p1.z()));
			} else {
				vv.push_back(osg::Vec3(q[i][0] + q[i][2], q[i][1] + q[i][3], p1.z()));
				for (int j = 1; j < segments; j++) {
					const float a = float(osg::PI_2) * j / segments;
					const float s = 1.0f - sinf(a), c = 1.0f - cosf(a);
					vv.push_back(osg::Vec3(
						q[i][0] + q[i][2] * s + q[i][4] * c,
						q[i][1] + q[i][3] * s + q[i][5] * c,
						p1.z()));
				}
				vv.push_back(osg::Vec3(q[i][0] + q[i][4], q[i][1] + q[i][5], p1.z()));
			}
		}

		return addPolygon(&(vv[0]), vv.size(), NULL);
	}

	SimpleGeometry::Face* SimpleGeometry::addEllipse(const osg::Vec3& center, const osg::Vec2& size, int segments) {
		std::vector<osg::Vec3> vv;

		for (int i = 0; i < segments; i++) {
			const float a = float(2 * osg::PI) * i / segments;
			vv.push_back(osg::Vec3(center.x() + size.x() * cosf(a),
				center.y() + size.y() * sinf(a),
				center.z()));
		}

		return addPolygon(&(vv[0]), vv.size(), NULL);
	}

	SimpleGeometry::Face* SimpleGeometry::addChord(const osg::Vec3& center, const osg::Vec2& size, float a1, float a2, int segments) {
		std::vector<osg::Vec3> vv;

		a2 -= a1;
		for (int i = 0; i <= segments; i++) {
			const float a = a1 + a2 * i / segments;
			vv.push_back(osg::Vec3(center.x() + size.x() * cosf(a),
				center.y() + size.y() * sinf(a),
				center.z()));
		}

		return addPolygon(&(vv[0]), vv.size(), NULL);
	}

	SimpleGeometry::Face* SimpleGeometry::addPie(const osg::Vec3& center, const osg::Vec2& size, float a1, float a2, int segments, const osg::Vec2& size2) {
		std::vector<osg::Vec3> vv;
		Triangulation* t = NULL;

		a2 -= a1;
		if (size2.x() >= 1E-6f && size2.y() >= 1E-6f) {
			t = new Triangulation;
			t->type = Triangulation::TRIANGLE_STRIP;
			t->indices.push_back(0);
			vv.push_back(osg::Vec3(center.x() + size2.x() * cosf(a1),
				center.y() + size2.y() * sinf(a1),
				center.z()));
			for (int i = 0; i <= segments; i++) {
				const float a = a1 + a2 * i / segments;
				vv.push_back(osg::Vec3(center.x() + size.x() * cosf(a),
					center.y() + size.y() * sinf(a),
					center.z()));
			}
			for (int i = segments; i > 0; i--) {
				const float a = a1 + a2 * i / segments;
				vv.push_back(osg::Vec3(center.x() + size2.x() * cosf(a),
					center.y() + size2.y() * sinf(a),
					center.z()));
			}
		} else {
			vv.push_back(center);
			for (int i = 0; i <= segments; i++) {
				const float a = a1 + a2 * i / segments;
				vv.push_back(osg::Vec3(center.x() + size.x() * cosf(a),
					center.y() + size.y() * sinf(a),
					center.z()));
			}
		}

		return addPolygon(&(vv[0]), vv.size(), t);
	}

	void SimpleGeometry::addPolyhedron(const osg::Vec3* vertices_, int vertexCount, const int* faceVertexIndices, const int* faceVertexCount, int faceCount, std::vector<Vertex*>* outVertices, std::vector<Face*>* outFaces) {
		std::vector<Vertex*> newVertices;

		// create new vertices
		for (int i = 0; i < vertexCount; i++) {
			Vertex *v = new Vertex;
			v->pos = vertices_[i];
			newVertices.push_back(v);
			if (outVertices) outVertices->push_back(v);
		}

		// create new faces
		std::map<std::pair<int, int>, Halfedge*> edgeMap;
		for (int i = 0, idx = 0; i < faceCount; i++) {
			Face *f = NULL;
			const int m = faceVertexCount[i];
			if (m >= 3) {
				f = new Face;
				int prevIndex = faceVertexIndices[idx + m - 1];
				Halfedge *prevEdge = NULL;

				for (int j = 0; j < m; j++) {
					int index = faceVertexIndices[idx + j];

					Halfedge *e = new Halfedge;
					if (j == 0) {
						// init first edge
						f->edge = e;
					} else {
						// set next edge of prev edge
						prevEdge->next = e;
					}

					// set end vertex
					e->vertex = newVertices[index];

					// set start vertex
					Vertex *prevVertex = newVertices[prevIndex];
					if (prevVertex->edge == NULL) prevVertex->edge = e;

					// set face
					e->face = f;

					// set opposite edge
					edgeMap[std::pair<int, int>(prevIndex, index)] = e;
					std::map<std::pair<int, int>, Halfedge*>::iterator it = edgeMap.find(std::pair<int, int>(index, prevIndex));
					if (it != edgeMap.end()) {
						e->opposite = it->second;
						it->second->opposite = e;
					}

					// over
					halfedges.push_back(e);
					prevIndex = index;
					prevEdge = e;
				}

				// set next edge of last edge
				prevEdge->next = f->edge;

				// add new face
				faces.push_back(f);
			}
			if (outFaces) outFaces->push_back(f);

			idx += m;
		}

		// add to existing data
		vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
	}

	void SimpleGeometry::addPyramid(const Face* src, bool isBipyramid, bool useFaceNormal, const osg::Vec3& p1, const osg::Vec3& p2) {
		// calculate face normal
		osg::Vec3 faceNormal, faceCenter;
		std::vector<Halfedge*> es;
		src->calculate(NULL, &faceNormal, &faceCenter, &es);
		faceNormal.normalize();
		const int m = es.size();

		// calculate apex coordinate
		osg::Vec3 pa[2] = { p1, p2 };
		if (useFaceNormal) {
			pa[0] = faceCenter + faceNormal*p1.z();
			pa[1] = faceCenter + faceNormal*p2.z();
		}
		const bool flipped = (pa[0] - faceCenter) * faceNormal < 0;

		std::vector<osg::Vec3> vv;
		std::vector<int> i1, i2;
		std::vector<Face*> newFaces;

		// add vertices
		for (int i = 0; i < m; i++) {
			vv.push_back(es[i]->vertex->pos);
		}
		vv.push_back(pa[0]);
		if (isBipyramid) vv.push_back(pa[1]);

		// add faces
		if (!isBipyramid) {
			i1.push_back(0);
			for (int i = 1; i < m; i++) {
				i1.push_back(flipped ? (m - i) : i);
			}
			i2.push_back(m);
		}
		int i0 = m - 1;
		for (int i = 0; i < m; i++) {
			if (flipped) {
				i1.push_back(i0); i1.push_back(i);
			} else {
				i1.push_back(i); i1.push_back(i0);
			}
			i1.push_back(m);
			i2.push_back(3);
			if (isBipyramid) {
				if (flipped) {
					i1.push_back(i); i1.push_back(i0);
				} else {
					i1.push_back(i0); i1.push_back(i);
				}
				i1.push_back(m + 1);
				i2.push_back(3);
			}
			i0 = i;
		}

		addPolyhedron(&(vv[0]), vv.size(), &(i1[0]), &(i2[0]), i2.size(),
			NULL, (!isBipyramid && src->triangulation.valid()) ? &newFaces : NULL);

		if (!newFaces.empty()) {
			newFaces[0]->triangulation = new Triangulation(*(src->triangulation.get()));
			if (flipped) (int&)(newFaces[0]->triangulation->type) ^= Triangulation::FLIPPED;
		}
	}

	void SimpleGeometry::addPyramid(const SimpleGeometry* src, bool isBipyramid, bool useFaceNormal, const osg::Vec3& p1, const osg::Vec3& p2) {
		if (src == NULL || src == this) return;
		for (size_t i = 0, m = src->faces.size(); i < m; i++) {
			if (src->faces[i]) addPyramid(src->faces[i], isBipyramid, useFaceNormal, p1, p2);
		}
	}

	void SimpleGeometry::addPrism(const Face* src, int antiprism, bool useFaceNormal, const osg::Vec3& p1, float scale, bool useEdgeNormal) {
		// calculate face normal
		osg::Vec3 faceNormal, faceCenter;
		std::vector<Halfedge*> es;
		src->calculate(NULL, &faceNormal, &faceCenter, &es);
		faceNormal.normalize();
		const int m = es.size();

		// calculate height
		osg::Vec3 ht = p1;
		if (useFaceNormal) {
			ht = faceNormal*p1.z();
		}
		const bool flipped = ht * faceNormal < 0;

		std::vector<osg::Vec3> vv, vtemp;
		std::vector<int> i1, i2;
		std::vector<Face*> newFaces;

		// calculate vertices
		vtemp.resize(m);
		if (useEdgeNormal) { //FIXME: not exactly
			int i0 = m - 1;
			for (int i = 0; i < m; i++) {
				osg::Vec3 nn = (es[i]->vertex->pos - es[i0]->vertex->pos) ^ faceNormal;
				nn.normalize();
				vtemp[i] += nn;
				vtemp[i0] += nn;
				i0 = i;
			}
			for (int i = 0; i < m; i++) {
				osg::Vec3 nn = vtemp[i];
				nn.normalize();
				vtemp[i] = es[i]->vertex->pos + nn*scale + ht;
			}
		} else {
			for (int i = 0; i < m; i++) {
				vtemp[i] = faceCenter + (es[i]->vertex->pos - faceCenter)*scale + ht;
			}
		}

		// add vertices
		for (int i = 0; i < m; i++) {
			vv.push_back(es[i]->vertex->pos);
		}
		if (antiprism == 0) {
			for (int i = 0; i < m; i++) {
				vv.push_back(vtemp[i]);
			}
		} else if (antiprism == 2 && m >= 3) {
			int i0 = m - 1, i_1 = m - 2;
			for (int i = 0; i < m; i++) {
				int i1 = (i + 1 < m) ? i + 1 : 0;
				vv.push_back((vtemp[i] + vtemp[i0])*0.5625f - (vtemp[i1] + vtemp[i_1])*0.0625f);
				i_1 = i0; i0 = i;
			}
		} else {
			int i0 = m - 1;
			for (int i = 0; i < m; i++) {
				vv.push_back((vtemp[i] + vtemp[i0])*0.5f);
				i0 = i;
			}
		}

		// add faces
		i1.push_back(0);
		for (int i = 1; i < m; i++) {
			i1.push_back(flipped ? (m - i) : i);
		}
		i2.push_back(m);

		i1.push_back(m);
		for (int i = 1; i < m; i++) {
			i1.push_back(m + (flipped ? i : (m - i)));
		}
		i2.push_back(m);

		int i0 = m - 1;
		for (int i = 0; i < m; i++) {
			if (antiprism) {
				if (flipped) {
					i1.push_back(i0); i1.push_back(i); i1.push_back(i + m);
					i1.push_back(i0); i1.push_back(i + m); i1.push_back(i0 + m);
				} else {
					i1.push_back(i); i1.push_back(i0); i1.push_back(i + m);
					i1.push_back(i0); i1.push_back(i0 + m); i1.push_back(i + m);
				}
				i2.push_back(3); i2.push_back(3);
			} else {
				if (flipped) {
					i1.push_back(i0); i1.push_back(i);
					i1.push_back(i + m); i1.push_back(i0 + m);
				} else {
					i1.push_back(i); i1.push_back(i0);
					i1.push_back(i0 + m); i1.push_back(i + m);
				}
				i2.push_back(4);
			}
			i0 = i;
		}

		addPolyhedron(&(vv[0]), vv.size(), &(i1[0]), &(i2[0]), i2.size(),
			NULL, (src->triangulation.valid()) ? &newFaces : NULL);

		if (!newFaces.empty()) {
			newFaces[0]->triangulation = new Triangulation(*(src->triangulation.get()));
			newFaces[1]->triangulation = new Triangulation(*(src->triangulation.get()));
			if (flipped) (int&)(newFaces[0]->triangulation->type) ^= Triangulation::FLIPPED;
			else (int&)(newFaces[1]->triangulation->type) ^= Triangulation::FLIPPED;
		}
	}

	void SimpleGeometry::addPrism(const SimpleGeometry* src, int antiprism, bool useFaceNormal, const osg::Vec3& p1, float shrink, bool useEdgeNormal) {
		if (src == NULL || src == this) return;
		for (size_t i = 0, m = src->faces.size(); i < m; i++) {
			if (src->faces[i]) addPrism(src->faces[i], antiprism, useFaceNormal, p1, shrink, useEdgeNormal);
		}
	}

	osg::Geometry* SimpleGeometry::createEdges(const osg::Vec3& color) {
		if (vertices.empty() || halfedges.empty()) return NULL;

		osg::ref_ptr<osg::Vec3Array> vv = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec3Array> cc = new osg::Vec3Array(osg::Array::BIND_OVERALL);
		cc->push_back(color);
		osg::ref_ptr<osg::DrawElementsUInt> ii = new osg::DrawElementsUInt(GL_LINES);

		// add vertices
		for (size_t i = 0, m = vertices.size(); i < m; i++) {
			Vertex *v = vertices[i];
			if (v) {
				v->tempIndex = vv->size(); // reset temp index
				vv->push_back(v->pos);
			}
		}

		// reset temp variable
		for (size_t i = 0, m = halfedges.size(); i < m; i++) {
			Halfedge *e = halfedges[i];
			if (e) e->temp = 0;
		}

		// for each halfedge
		//FIXME: it doesn't work if the halfedge is not circular (only occurs when there is an incomplete face)
		for (size_t i = 0, m = halfedges.size(); i < m; i++) {
			Halfedge *e = halfedges[i];
			if (e && e->next && e->next->temp == 0) {
				e->next->temp = 1;
				if (e->next->opposite) e->next->opposite->temp = 1;
				if (e->vertex && e->next->vertex) {
					ii->push_back(e->vertex->tempIndex);
					ii->push_back(e->next->vertex->tempIndex);
				}
			}
		}

		if (vv->empty() || ii->empty()) return NULL;

		// debug
		UTIL_INFO "Edge count: " << (ii->size() / 2) << std::endl;

		// over
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		geom->setVertexArray(vv);
		geom->setColorArray(cc);
		geom->addPrimitiveSet(ii);
		return geom.release();
	}

	osg::Geometry* SimpleGeometry::createFaces(const osg::Vec3& color, bool useFaceCenter, bool useWeightedFaceNormal) {
		if (vertices.empty() || halfedges.empty() || faces.empty()) return NULL;

		osg::ref_ptr<osg::Vec3Array> vv = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec3Array> nn = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec3Array> cc = new osg::Vec3Array(osg::Array::BIND_OVERALL);
		cc->push_back(color);
		osg::ref_ptr<osg::DrawElementsUInt> ii = new osg::DrawElementsUInt(GL_TRIANGLES);

		// add vertices
		for (size_t i = 0, m = vertices.size(); i < m; i++) {
			Vertex *v = vertices[i];
			if (v) {
				v->tempIndex = vv->size(); // reset temp index
				vv->push_back(v->pos);
				nn->push_back(osg::Vec3());
			}
		}

		// for each face
		for (size_t i = 0, mf = faces.size(); i < mf; i++) {
			Face *f = faces[i];
			if (f == NULL || f->edge==NULL) continue;

			// calculate face normal and vertex count
			std::vector<Halfedge*> es;
			osg::Vec3 faceNormal;
			f->calculate(NULL, &faceNormal, NULL, &es);
			faceNormal *= -1; //???

			const size_t m = es.size();
			if (m < 3) continue;

			// add face normal to each vertex
			//TODO: other normal type
			if (!useWeightedFaceNormal) faceNormal.normalize();
			for (size_t i = 0; i < m; i++) {
				(*nn)[es[i]->vertex->tempIndex] += faceNormal;
			}

			// add face center if necessary
			int centerIndex = -1;
			if (useFaceCenter && m >= 4) {
				osg::Vec3 p;
				for (size_t i = 0; i < m; i++) {
					p += es[i]->vertex->pos;
				}
				centerIndex = vv->size();
				vv->push_back(p / m);
				nn->push_back(faceNormal);
			}

			// add triangles of this face
			f->triangulation->addTriangulation(es, ii.get(), centerIndex);
		}

		// normalize normals
		for (size_t i = 0, m = nn->size(); i < m; i++) {
			(*nn)[i].normalize();
		}

		if (vv->empty() || ii->empty()) return NULL;

		// debug
		UTIL_INFO "Triangle count: " << (ii->size() / 3) << std::endl;

		// over
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		geom->setVertexArray(vv);
		geom->setNormalArray(nn);
		geom->setColorArray(cc);
		geom->addPrimitiveSet(ii);
		return geom.release();
	}

	void SimpleGeometry::applyTransform(const osg::Matrix& mat) {
		for (size_t i = 0, m = vertices.size(); i < m; i++) {
			Vertex *v = vertices[i];
			if (v) {
				osg::Vec4 v4 = osg::Vec4(v->pos, 1) * mat;
				v->pos.x() = v4.x();
				v->pos.y() = v4.y();
				v->pos.z() = v4.z();
			}
		}
	}

	void SimpleGeometry::addSimpleGeometry(SimpleGeometry* src) {
		if (src == NULL || src == this) return;

		for (size_t i = 0, m = src->vertices.size(); i < m; i++) {
			if (src->vertices[i]) vertices.push_back(src->vertices[i]);
		}
		for (size_t i = 0, m = src->halfedges.size(); i < m; i++) {
			if (src->halfedges[i]) halfedges.push_back(src->halfedges[i]);
		}
		for (size_t i = 0, m = src->faces.size(); i < m; i++) {
			if (src->faces[i]) faces.push_back(src->faces[i]);
		}

		src->vertices.clear();
		src->halfedges.clear();
		src->faces.clear();
	}

}
