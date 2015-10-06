#pragma once

#include <osg/Node>
#include <osg/StateSet>
#include "util_object.h"

class XMLNode;

namespace gfx {

	class Appearance :
		public osg::Object
	{
	public:
		/// the node type
		enum Type {
			APPEARANCE,
			SHADER,
			MESH_CUBE, //!< ad-hoc
		};
	protected:
		virtual ~Appearance();
	public:
		META_Object(gfx, Appearance);

		Appearance();
		Appearance(const Appearance& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		/** get or create instance.
		\param shape The shape, see \ref game::MapData::MapShape.
		*/
		osg::Node* getOrCreateInstance(int shape);

		bool load(const XMLNode* node); //!< load from XML node

	public:
		int type; //!< the node \ref Type.

		// if node type is SAHDER

		osg::Vec4 ambient; //!< ambient color
		osg::Vec4 diffuse; //!< diffuse color
		osg::Vec4 specular; //!< specular color
		osg::Vec4 emissive; //!< emission color
		float specularHardness; //!< specular hardness

		// if node type is MESH_CUBE

		osg::Vec3 pos; //!< position.
		osg::Vec3 rot; //!< rotation (yaw, pitch, roll)
		osg::Vec3 size; //!< size
		osg::Vec3 center;
		float bevel;
		bool solid;
		bool wireframe;
		bool lod;
		osg::Vec3 solidColor;
		osg::Vec3 wireframeColor;

		std::vector<osg::ref_ptr<Appearance> > subNodes; //!< subnodes

	public:
		UTIL_ADD_BYVAL_GETTER_SETTER(int, type);

		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, ambient);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, diffuse);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, specular);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec4, emissive);
		UTIL_ADD_BYVAL_GETTER_SETTER(float, specularHardness);

		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, rot);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, size);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3, center);
		UTIL_ADD_BYVAL_GETTER_SETTER(float, bevel);
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
