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

	class Triangulation;

	/// represents a simple polygon mesh using halfedge structure

	class SimpleGeometry : public osg::Object {
	protected:
		virtual ~SimpleGeometry();
	public:
		META_Object(gfx, SimpleGeometry);

		SimpleGeometry();
		SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		void clear();

		/** add a polygon
		\param vertices The array of vertices
		\param vertexCount The vertex count
		\param triangulation The optional triangulation
		*/
		void addPolygon(const osg::Vec3* vertices, int vertexCount, Triangulation* triangulation);

		/** add a (rounded) rectangle
		\param p1 A corner
		\param p2 Another corner, assume p1 and p2 have the same z coordinate
		\param bevel The bevel size
		\param segments The bevel segments
		*/
		void addRect(const osg::Vec3& p1, const osg::Vec3& p2, float bevel, int segments);

		/** add an ellipse */
		void addEllipse(const osg::Vec3& center, const osg::Vec2& size, int segments);

		/** add a circle */
		void addCircle(const osg::Vec3& center, float size, int segments) {
			addEllipse(center, osg::Vec2(size, size), segments);
		}

		/** add a polyhedron
		\param vertices The array of vertices
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

		/** create a wireframe geometry
		\param color The color
		*/
		osg::Geometry* createEdges(const osg::Vec3& color);

		/** create a solid geometry
		\param color The color
		\param useFaceCenter Create a face center if the face has >=4 vertices and doesn't have a triangulator
		\param useWeightedFaceNormal Use face area as weight for face normal
		*/
		osg::Geometry* createFaces(const osg::Vec3& color, bool useFaceCenter, bool useWeightedFaceNormal);

	public:
		struct Vertex;
		struct Halfedge;
		struct Face;

	private:
		std::vector<Vertex*> vertices;
		std::vector<Halfedge*> halfedges;
		std::vector<Face*> faces;
	};

	/// represents a simple triangulation method (internal class)

	class Triangulation : public osg::Object {
	public:
		/// the triangulation type
		enum TriangulationType {
			TRIANGLES = GL_TRIANGLES, //!< a list of triangles.
			TRIANGLE_STRIP = GL_TRIANGLE_STRIP, //!< triangle strip (in this case \ref indices can have only 1 element).
			TRIANGLE_FAN = GL_TRIANGLE_FAN, //!< triangle fan (default value, in this case \ref indices can have only 1 element).
			QUADS = GL_QUADS, //!< a list of quads.
			TYPE_MASK = 0xFF,
			FLIPPED = 0x100,
		};
	protected:
		virtual ~Triangulation();
	public:
		META_Object(gfx, Triangulation);

		Triangulation();
		Triangulation(const Triangulation& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		/** add triangulation of this face (internal function)
		\param[in] es The halfedges of this face
		\param[out] ii The vertex index list of triangles
		\param[in] centerIndex The vertex index of center of this face (will overwrite \ref type), -1 means no center
		*/
		void addTriangulation(const std::vector<SimpleGeometry::Halfedge*>& es, osg::DrawElementsUInt* ii, int centerIndex) const;

		bool isFlipped() const {
			return (type & FLIPPED) != 0;
		}

	public:
		TriangulationType type; //!< the \ref TriangulationType.
		std::vector<int> indices; //!< the indices of triangle vertices, whose structure depends on the value of \ref type.

	private:
		static int flip(int index, int size) {
			return index ? (size - index) : 0;
		}
		int flipIfNecessary(int index, int size) const {
			return (type & FLIPPED) ? flip(index, size) : index;
		}
	};

}
