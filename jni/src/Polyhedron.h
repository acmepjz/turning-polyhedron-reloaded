#pragma once

#include <osg/Referenced>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>
#include "MapData.h"

namespace game {

	class ObjectType;

	class Polyhedron :
		public osg::Referenced
	{
	protected:
		virtual ~Polyhedron();
	public:
		Polyhedron();

	public:
		std::string id; //!< the polyhedron id

		ObjectType* objType; //!< the object type, NULL = default
		int flags; //!< the polyhedron flags
		MapPosition pos; //!< the start position

		osg::ref_ptr<osg::Node> appearance; //!< the appearance
	};

}
