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

	bool ObjectTypeMap::add(ObjectType* obj){
		if (!obj || obj->name.empty()) {
			OSG_NOTICE << "[" __FUNCTION__ "] object doesn't have id" << std::endl;
			return false;
		}

		IdMap::iterator it = map.find(obj->name);
		if (it != map.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] object name '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
		}

		map[obj->name] = obj;
		return true;
	}

	ObjectType* ObjectTypeMap::lookup(const std::string& name){
		if (name.empty() || name == "default") return NULL;
		IdMap::iterator it = map.find(name);
		if (it == map.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] name '" << name << "' not found" << std::endl;
			return NULL;
		}
		return it->second.get();
	}

	void ObjectTypeMap::init(){
		for (IdMap::iterator it = map.begin(); it != map.end(); ++it) {
			it->second->init(this);
		}
	}

	REG_OBJ_WRAPPER(game, ObjectTypeMap, "")
	{
		ADD_MAP_SERIALIZER(map, ObjectTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
