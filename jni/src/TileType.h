#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/Vec2i>
#include <string>
#include "ObjectType.h"
#include "util.h"

namespace game {

	/// Represents a tile type.

	class TileType :
		public osg::Object
	{
	protected:
		virtual ~TileType();
	public:
		META_Object(game, TileType);

		TileType();
		TileType(const TileType& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		std::string id; //!< id, used to find this tile type
		int index; //!< the (permanent) index (optional), used to find this tile type, 0 = no index

		std::string objType; //!< the object type

		/** z coordinate of blocked hit test area. The format is (lower,upper).
		For example ground=(-1,0), wall=(-1,1), non-block=(0,0).
		\note Note that normal hit test area is fixed at z=0.
		*/
		osg::Vec2i blockedArea;

		std::string name; //!< the gettext'ed object name
		std::string desc; //!< the gettext'ed object description

		osg::ref_ptr<osg::Node> appearance; //!< the appearance

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, index);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec2i, blockedArea);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, desc);
		UTIL_ADD_OBJ_GETTER_SETTER(osg::Node, appearance);
	};

}
