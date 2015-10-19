#pragma once

#include "util_object.h"
#include <osg/Geometry>
#include <set>

namespace gfx {

	/** create a cube (ad-hoc)
	\param p1 A corner
	\param p2 Another corner
	\param wireframe Is it wireframe
	\param bevel The bevel size
	\param color The color
	*/
	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color);

	/// represents a polygon mesh using halfedge structure

	class SimpleGeometry : public osg::Object {
	protected:
		virtual ~SimpleGeometry();
	public:
		META_Object(gfx, SimpleGeometry);

		SimpleGeometry();
		SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		void clear();

		/** add a polyhedron
		\param vertices The array vertices
		\param vertexCount The vertex count
		\param faceVertexIndices The array of index of vertices of faces, e.g. 0,1,2,3,0,3,4,5, ...
		\param faceVertexCount The array of face vertex count, e.g. 4,4,4 ...
		\param faceCount The face count
		*/
		void addPolyhedron(const osg::Vec3* vertices, int vertexCount, const int* faceVertexIndices, const int* faceVertexCount, int faceCount);

		/** add a cube
		\param p1 A corner
		\param p2 Another corner
		\param bevel The bevel size
		*/
		void addCube(const osg::Vec3& p1, const osg::Vec3& p2, float bevel);

		osg::Geometry* createEdges(const osg::Vec3& color);
		osg::Geometry* createFaces(const osg::Vec3& color);

	private:
		struct Vertex;
		struct Halfedge;
		struct Face;

		std::vector<Vertex*> vertices;
		std::vector<Halfedge*> halfedges;
		std::vector<Face*> faces;
	};

}
