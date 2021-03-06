#pragma once

#include "util_object.h"
#include <osg/Geometry>
#include <set>

class XMLNode;

namespace gfx {

#if 0
	/** create a cube (ad-hoc)
	\param p1 A corner
	\param p2 Another corner
	\param wireframe Is it wireframe
	\param bevel The bevel size
	\param color The color
	*/
	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color);
#endif

	class Triangulation;

	/// represents a simple polygon mesh using halfedge structure

	class SimpleGeometry : public osg::Object {
	public:
		struct Vertex;
		struct Halfedge;
		struct Face;

	protected:
		virtual ~SimpleGeometry();
	public:
		META_Object(gfx, SimpleGeometry);

		SimpleGeometry();
		SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		/// clear all vertices, edges and faces
		void clear();

		/// apply transform
		void applyTransform(const osg::Matrix& mat);

		/// transfer the source geometry to this geometry (after calling this function contents in `src` is cleared).
		void addSimpleGeometry(SimpleGeometry* src);

		/** add a polygon
		\param vertices The array of vertices
		\param vertexCount The vertex count
		\param triangulation The optional triangulation
		*/
		Face* addPolygon(const osg::Vec3* vertices, int vertexCount, Triangulation* triangulation);

		/** add a (rounded) rectangle
		\param p1 A corner
		\param p2 Another corner, assume p1 and p2 have the same z coordinate
		\param bevel The bevel size
		\param segments The number of line segments in beveled corner
		*/
		Face* addRect(const osg::Vec3& p1, const osg::Vec3& p2, float bevel, int segments);

		/** add an ellipse
		\param center The center
		\param size The size
		\param segments The number of line segments
		*/
		Face* addEllipse(const osg::Vec3& center, const osg::Vec2& size, int segments);

		/** add a circle
		\param center The center
		\param size The size
		\param segments The number of line segments
		*/
		Face* addCircle(const osg::Vec3& center, float size, int segments) {
			return addEllipse(center, osg::Vec2(size, size), segments);
		}

		/** add a chord
		\param center The center
		\param size The size
		\param a1 The start angle (in radian)
		\param a2 The end angle (in radian)
		\param segments The number of line segments
		*/
		Face* addChord(const osg::Vec3& center, const osg::Vec2& size, float a1, float a2, int segments);

		/** add a chord
		\param center The center
		\param size The size
		\param a1 The start angle (in radian)
		\param a2 The end angle (in radian)
		\param segments The number of line segments
		*/
		Face* addChord(const osg::Vec3& center, float size, float a1, float a2, int segments) {
			return addChord(center, osg::Vec2(size, size), a1, a2, segments);
		}

		/** add a pie
		\param center The center
		\param size The size
		\param a1 The start angle (in radian)
		\param a2 The end angle (in radian)
		\param segments The number of line segments
		\param size2 The inner size, used to create an annular pie
		*/
		Face* addPie(const osg::Vec3& center, const osg::Vec2& size, float a1, float a2, int segments, const osg::Vec2& size2 = osg::Vec2());

		/** add a pie
		\param center The center
		\param size The size
		\param a1 The start angle (in radian)
		\param a2 The end angle (in radian)
		\param segments The number of line segments
		\param size2 The inner size, used to create an annular pie
		*/
		Face* addPie(const osg::Vec3& center, float size, float a1, float a2, int segments, float size2 = 0.0f) {
			return addPie(center, osg::Vec2(size, size), a1, a2, segments, osg::Vec2(size2, size2));
		}

		/** add a polyhedron
		\param[in] vertices The array of vertices
		\param[in] vertexCount The vertex count
		\param[in] faceVertexIndices The array of index of vertices of faces, e.g. 0,1,2,3,0,3,4,5, ...
		\param[in] faceVertexCount The array of face vertex count, e.g. 4,4,4 ...
		\param[in] faceCount The face count
		\param[out] outVertices The vertices added
		\param[out] outFaces The faces added
		*/
		void addPolyhedron(const osg::Vec3* vertices, int vertexCount, const int* faceVertexIndices, const int* faceVertexCount, int faceCount, std::vector<Vertex*>* outVertices = NULL, std::vector<Face*>* outFaces = NULL);

		/** add a cube
		\param p1 A corner
		\param p2 Another corner
		\param bevel The bevel size
		*/
		void addCube(const osg::Vec3& p1, const osg::Vec3& p2, float bevel);

		/** add a pyramid or bipyramid
		\param src The source
		\param isBipyramid Is it a bipyramid, `true` then `p2` is used
		\param useFaceNormal Use face normal to determine the height, `true` then only z coordinate of `p1` and `p2` is used
		\param p1 The apex coordinate
		\param p2 The apex coordinate (only used if it is a bipyramid)
		*/
		void addPyramid(const Face* src, bool isBipyramid, bool useFaceNormal, const osg::Vec3& p1, const osg::Vec3& p2);
		void addPyramid(const SimpleGeometry* src, bool isBipyramid, bool useFaceNormal, const osg::Vec3& p1, const osg::Vec3& p2);

		/** add a prism or antiprism
		\param src The source
		\param antiprism 0=prism, 1=antiprism with linear interpolated coordinate, 2=cubic interolated
		\param useFaceNormal Use face normal to determine the height, `true` then only z coordinate of `p1` is used
		\param p1 The height vector
		\param scale The scale factor or expand length if `useEdgeNormal` is true.
		\param useEdgeNormal Use edge normal to determine the scale.
		*/
		void addPrism(const Face* src, int antiprism, bool useFaceNormal, const osg::Vec3& p1, float scale = 1.0f, bool useEdgeNormal = false);
		void addPrism(const SimpleGeometry* src, int antiprism, bool useFaceNormal, const osg::Vec3& p1, float scale = 1.0f, bool useEdgeNormal = false);

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

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `triangulation`

		bool valid(int vertexCount) const; //!< check if vertex index is valid

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
