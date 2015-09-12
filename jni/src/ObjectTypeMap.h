#pragma once

#include <osg/Object>
#include <osg/ref_ptr>
#include <string>
#include <map>
#include "ObjectType.h"
#include "util.h"

namespace game {

	/// A map used to look up object type

	class ObjectTypeMap : public osg::Object {
	protected:
		virtual ~ObjectTypeMap();
	public:
		META_Object(game, ObjectTypeMap);

		ObjectTypeMap();
		ObjectTypeMap(const ObjectTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool add(ObjectType* obj); //!< Add an object to map (which must has a valid id).
		ObjectType* lookup(const std::string& name); //!< Find an object with given name.

		typedef std::map<std::string, osg::ref_ptr<ObjectType> > IdMap;
		IdMap map;

		UTIL_ADD_BYREF_GETTER_SETTER(IdMap, map);
	};

}

