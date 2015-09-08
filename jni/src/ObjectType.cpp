#include "ObjectType.h"
#include "Interaction.h"
#include "util.h"
#include <osg/Notify>

namespace game {

	ObjectType::ObjectType()
	{

	}

	ObjectType::ObjectType(const ObjectType& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, desc(other.desc)
	{
		util::copyMap(interactions, other.interactions, copyop);
	}

	ObjectType::~ObjectType(){

	}

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

}
