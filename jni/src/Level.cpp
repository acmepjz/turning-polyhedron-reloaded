#include "Level.h"
#include <osgDB/ObjectWrapper>

namespace game {

	Level::Level()
	{
	}

	Level::Level(const Level& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, solution(other.solution)
		, tileTypeMap(util::copyObj(other.tileTypeMap.get(), copyop)) //always deep copy
		, objectTypeMap(util::copyObj(other.objectTypeMap.get(), copyop)) //always deep copy
	{
		//following objects are always deep copy
		util::copyMap(maps, other.maps, copyop, true);
		util::copyMap(polyhedra, other.polyhedra, copyop, true);
	}

	Level::~Level()
	{
	}

	REG_OBJ_WRAPPER(game, Level, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(solution, "");
		ADD_MAP_SERIALIZER(maps, Level::MapDataMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_MAP_SERIALIZER(polyhedra, Level::PolyhedronMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_OBJECT_SERIALIZER(tileTypeMap, TileTypeMap, NULL);
		ADD_OBJECT_SERIALIZER(objectTypeMap, ObjectTypeMap, NULL);
	}
}
