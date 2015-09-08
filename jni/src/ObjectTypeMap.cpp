#include "ObjectTypeMap.h"
#include "ObjectType.h"
#include "util.h"
#include <osg/Notify>
#include <osgDB/ObjectWrapper>

namespace game {

	ObjectTypeMap::ObjectTypeMap(){

	}

	ObjectTypeMap::ObjectTypeMap(const ObjectTypeMap& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
	{
		util::copyMap(map, other.map, copyop);
	}

	ObjectTypeMap::~ObjectTypeMap(){

	}

	ObjectType* ObjectTypeMap::lookup(const std::string& name){
		std::map<std::string, osg::ref_ptr<ObjectType> >::iterator it = map.find(name);
		if (it == map.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] name '" << name << "' not found" << std::endl;
			return NULL;
		}
		return it->second.get();
	}

	REG_OBJ_WRAPPER(game, ObjectTypeMap, "")
	{
		ADD_MAP_SERIALIZER(map, ObjectTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
