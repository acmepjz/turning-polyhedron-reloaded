#include "TileType.h"
#include "util.h"
#include <osg/Node>
#include <osg/Notify>
#include <stdlib.h>
#include <osgDB/ObjectWrapper>

namespace game {

	TileType::TileType()
		: index(0)
		, blockedArea(-1, 0)
	{
	}

	TileType::TileType(const TileType& other, const osg::CopyOp& copyop)
		: Object(other,copyop)
		, id(other.id)
		, index(other.index)
		, objType(other.objType)
		, blockedArea(other.blockedArea)
		, name(other.name)
		, desc(other.desc)
		, appearance(copyop(other.appearance))
	{
	}


	TileType::~TileType()
	{
	}

	REG_OBJ_WRAPPER(game, TileType, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(index, 0);
		ADD_STRING_SERIALIZER(objType, "");
		ADD_VEC2I_SERIALIZER(blockedArea, osg::Vec2i(-1, 0));
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_OBJECT_SERIALIZER(appearance, osg::Node, NULL);
	}

}
