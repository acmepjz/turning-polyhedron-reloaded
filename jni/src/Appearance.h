#pragma once

#include <osg/Node>
#include <osg/StateSet>
#include "util_object.h"

class XMLNode;

namespace gfx {

	class SimpleGeometry;
	class Triangulation;

	/// The appearance node (experimental)

	class Appearance :
		public osg::Object
	{
	public:
		/// the node type
		enum Type {
			APPEARANCE,
			SHADER,
			TRANSFORM,
			MESH,
		};
		/// the mesh type
		enum MeshType {
			POLYHEDRON = 0,
			CUBE,
			POLYGON = 0x100,
			RECTANGLE,
			ELLIPSE,
			CHORD,
			PIE,
			PRISM = 0x200,
			PYRAMID,
		};
	protected:
		virtual ~Appearance();
	public:
		META_Object(gfx, Appearance);

		Appearance();
		Appearance(const Appearance& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		/** create a \ref SimpleGeometry if the node type is \ref MESH.
		\param existing The existing geometry (optional).
		\param shape The shape, see \ref game::MapData::MapShape.
		\param isLODed Is it less detailed
		*/
		SimpleGeometry* createSimpleGeometry(SimpleGeometry* existing, int shape, bool isLODed) const;

		osg::Geode* createGeodeFromSimpleGeometry(SimpleGeometry* sg) const;

		/** check if the node has LOD if the node type is \ref MESH. */
		bool hasLOD() const;

		/** get or create instance.
		\param shape The shape, see \ref game::MapData::MapShape.
		*/
		osg::Node* getOrCreateInstance(int shape);

		bool load(const XMLNode* node); //!< load from XML node

		void loadVertices(const XMLNode* node); //!< (internal function) load vertices, assume the node name is `vertices`.
		void loadFaces(const XMLNode* node); //!< (internal function) load faces, assume the node name is `faces`.

	public:
		int type; //!< the node \ref Type.

		// if node type is SAHDER

		osg::Vec4 ambient; //!< ambient color
		osg::Vec4 diffuse; //!< diffuse color
		osg::Vec4 specular; //!< specular color
		osg::Vec4 emissive; //!< emission color
		float specularHardness; //!< specular hardness

		// if node type is MESH

		int meshType; //!< the mesh type. \sa MeshType
		osg::Vec3 pos; //!< position.
		osg::Vec3 rot; //!< rotation (yaw, pitch, roll)
		osg::Vec3 scale; //!< scale or size
		osg::Vec3 scale2;
		osg::Vec2 angles;
		osg::Vec3 center;
		float bevel; //!< bevel size
		int segments;
		bool solid; //!< set to draw solid or not
		bool wireframe; //!< set to draw wireframe or not
		bool lod; //!< set to use LOD or not (used when \ref bevel > 0)
		osg::Vec3 solidColor; //!< color
		osg::Vec3 wireframeColor; //!< wireframe color

		std::vector<float> _vertices;
		std::vector<int> _faces;
		osg::ref_ptr<Triangulation> _triangulation;

		std::vector<osg::ref_ptr<Appearance> > subNodes; //!< subnodes

	public:
		UTIL_ADD_BYVAL_GETTER_SETTER(int, type);

		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, ambient);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, diffuse);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, specular);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, emissive);
		UTIL_ADD_BYVAL_GETTER_SETTER(float, specularHardness);

		UTIL_ADD_BYVAL_GETTER_SETTER(int, meshType);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, rot);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, scale);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, scale2);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec2, angles);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, center);
		UTIL_ADD_BYVAL_GETTER_SETTER(float, bevel);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, segments);
		UTIL_ADD_BYVAL_GETTER_SETTER(bool, solid);
		UTIL_ADD_BYVAL_GETTER_SETTER(bool, wireframe);
		UTIL_ADD_BYVAL_GETTER_SETTER(bool, lod);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, solidColor);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, wireframeColor);

		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<Appearance> >, subNodes);

	public:
		//the following properties don't save to file and is generated at runtime
		std::map<int, osg::ref_ptr<osg::Node> > _instances;
		osg::ref_ptr<osg::StateSet> _stateSet;
	};

}
