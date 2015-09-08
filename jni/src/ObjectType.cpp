#include "ObjectType.h"
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

	REG_OBJ_WRAPPER(game, ObjectType, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_MAP_SERIALIZER(interactions, ObjectType::InteractionMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
