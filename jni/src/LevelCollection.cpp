#include "LevelCollection.h"
#include <osgDB/ObjectWrapper>

namespace game {

	LevelCollection::LevelCollection()
	{
	}

	LevelCollection::LevelCollection(const LevelCollection& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, tileTypeMap(util::copyObj(other.tileTypeMap.get(), copyop)) //always deep copy
		, objectTypeMap(util::copyObj(other.objectTypeMap.get(), copyop)) //always deep copy
	{
		//following objects are always deep copy
		util::copyVector(levels, other.levels, copyop, true);
	}

	LevelCollection::~LevelCollection()
	{
	}

	REG_OBJ_WRAPPER(game, LevelCollection, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_OBJECT_SERIALIZER(tileTypeMap, TileTypeMap, NULL);
		ADD_OBJECT_SERIALIZER(objectTypeMap, ObjectTypeMap, NULL);
		ADD_VECTOR_SERIALIZER(levels, std::vector<osg::ref_ptr<Level> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}
}
