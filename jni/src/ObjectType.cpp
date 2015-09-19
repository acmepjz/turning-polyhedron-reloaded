#include "ObjectType.h"
#include "ObjectTypeMap.h"
#include "Interaction.h"
#include "util.h"
#include <osg/Notify>
#include <osgDB/ObjectWrapper>

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

	void ObjectType::init(ObjectTypeMap* otm){
		_interactions.clear();
		for (InteractionMap::iterator it = interactions.begin(); it != interactions.end(); ++it) {
			if (it->first.empty() || it->first == "default") {
				_interactions[NULL] = it->second;
			} else {
				ObjectType* type2 = otm->lookup(it->first);
				if (type2) _interactions[type2] = it->second;
			}
		}
	}

	REG_OBJ_WRAPPER(game, ObjectType, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_MAP_SERIALIZER(interactions, ObjectType::InteractionMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
