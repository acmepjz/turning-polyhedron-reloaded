#include "ObjectType.h"
#include "Interaction.h"
#include <osg/Notify>

namespace game {

	ObjectType::ObjectType()
	{

	}

	ObjectType::~ObjectType(){

	}

	ObjectTypeMap::ObjectTypeMap(){

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
