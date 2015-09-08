#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>
#include "MapData.h"
#include "util.h"

namespace game {

	class ObjectType;

	class Polyhedron :
		public osg::Object
	{
	protected:
		virtual ~Polyhedron();
	public:
		META_Object(game, Polyhedron);

		Polyhedron();
		Polyhedron(const Polyhedron& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

	public:
		std::string id; //!< the polyhedron id

		std::string objType; //!< the object type
		int flags; //!< the polyhedron flags
		MapPosition pos; //!< the start position

		osg::ref_ptr<osg::Node> appearance; //!< the appearance

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, flags);
		UTIL_ADD_MapPosition_GETTER_SETTER(pos);
		UTIL_ADD_OBJ_GETTER_SETTER(osg::Node, appearance);
	};

}
