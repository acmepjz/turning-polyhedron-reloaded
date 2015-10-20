#include "SimpleGeometry.h"
#include "util_err.h"

namespace gfx {

	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color) {
#if 1
		// test only
		osg::ref_ptr<SimpleGeometry> geom = new SimpleGeometry;
		geom->addCube(p1, p2, bevel);
		return wireframe ? geom->createEdges(color) : geom->createFaces(color);
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

	void SimpleGeometry::addCube(const osg::Vec3& p1, const osg::Vec3& p2, float bevel) {
		osg::Vec3 p[2] = { p1, p2 };
		for (int i = 0; i < 2; i++) {
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

	void Triangulation::addTriangulation(const std::vector<SimpleGeometry::Halfedge*>& es, osg::DrawElementsUInt* ii) const {
		if (this) {
			switch (type) {
			case TRIANGLES:
				if (indices.size() >= 3) {
					for (int i = 0, m = indices.size(); i + 3 <= m; i += 3) {
						ii->push_back(es[indices[i]]->vertex->tempIndex);
						ii->push_back(es[indices[i + 1]]->vertex->tempIndex);
						ii->push_back(es[indices[i + 2]]->vertex->tempIndex);
					}
					return;
				}
				break;
			case TRIANGLE_STRIP:
				if (indices.size() >= 3) {
					int i0 = es[indices[0]]->vertex->tempIndex;
					int i1 = es[indices[1]]->vertex->tempIndex;
					for (int i = 2, m = indices.size(); i < m; i++) {
						if (i & 1) {
							ii->push_back(i1); ii->push_back(i0);
						} else {
							ii->push_back(i0); ii->push_back(i1);
						}
						int i2 = es[indices[i]]->vertex->tempIndex;
						ii->push_back(i2);
						i0 = i1; i1 = i2;
					}
					return;
				} else if (indices.size() >= 1) {
					const int m = es.size();
					int idxStart = indices[0];
					if (idxStart < 0 || idxStart >= m) idxStart = 0;
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
					int i0 = es[indices[0]]->vertex->tempIndex;
					int i1 = es[indices[1]]->vertex->tempIndex;
					for (int i = 2, m = indices.size(); i < m; i++) {
						int i2 = es[indices[i]]->vertex->tempIndex;
						ii->push_back(i0); ii->push_back(i1); ii->push_back(i2);
						i1 = i2;
					}
					return;
				} else if (indices.size() >= 1) {
					const int m = es.size();
					int idxStart = indices[0];
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
						int i0 = es[indices[i]]->vertex->tempIndex,
							i1 = es[indices[i + 1]]->vertex->tempIndex,
							i2 = es[indices[i + 2]]->vertex->tempIndex,
							i3 = es[indices[i + 3]]->vertex->tempIndex;
						ii->push_back(i0); ii->push_back(i1); ii->push_back(i2);
						ii->push_back(i0); ii->push_back(i2); ii->push_back(i3);
					}
					return;
				}
				break;
			}
		}

		// default implementation
		int i0 = es[0]->vertex->tempIndex;
		int i1 = es[1]->vertex->tempIndex;
		for (int i = 2, m = es.size(); i < m; i++) {
			int i2 = es[i]->vertex->tempIndex;
			ii->push_back(i0); ii->push_back(i1); ii->push_back(i2);
			i1 = i2;
		}
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

	void SimpleGeometry::addPolyhedron(const osg::Vec3* vertices_, int vertexCount, const int* faceVertexIndices, const int* faceVertexCount, int faceCount) {
		std::vector<Vertex*> newVertices;

		// create new vertices
		for (int i = 0; i < vertexCount; i++) {
			Vertex *v = new Vertex;
			v->pos = vertices_[i];
			newVertices.push_back(v);
		}

		// create new faces
		std::map<std::pair<int, int>, Halfedge*> edgeMap;
		for (int i = 0, idx = 0; i < faceCount; i++) {
			const int m = faceVertexCount[i];
			if (m >= 3) {
				Face *f = new Face;
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

			idx += m;
		}

		// add to existing data
		vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
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

	osg::Geometry* SimpleGeometry::createFaces(const osg::Vec3& color) {
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
		for (size_t i = 0, m = faces.size(); i < m; i++) {
			Face *f = faces[i];
			if (f == NULL) continue;
			Halfedge *e0 = f->edge;
			if (e0 == NULL) continue;

			// calculate face normal and vertex count
			osg::Vec3 faceNormal, prevVector;
			std::vector<Halfedge*> es;
			es.push_back(e0);
			for (Halfedge* e = e0->next; e != e0; e = e->next) {
				es.push_back(e);

				osg::Vec3 currentVector = e->vertex->pos - e0->vertex->pos;
				if (es.size() >= 3) faceNormal += currentVector ^ prevVector;
				prevVector = currentVector;
			}

			if (es.size() < 3) continue;

			// add face normal to each vertex
			//TODO: other normal type
			//faceNormal.normalize();
			for (size_t i = 0, m = es.size(); i < m; i++) {
				(*nn)[es[i]->vertex->tempIndex] += faceNormal;
			}

			// add triangles of this face
			f->triangulation->addTriangulation(es, ii.get());
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

}
