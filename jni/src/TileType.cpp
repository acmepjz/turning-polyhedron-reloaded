#include "TileType.h"
#include "ObjectTypeMap.h"
#include "TileTypeMap.h"
#include "util.h"
#include <osg/Node>
#include <osg/Notify>
#include <stdlib.h>
#include <osgDB/ObjectWrapper>

namespace game {

	TileType::TileType()
		: index(0)
		, flags(0)
		, blockedArea(-1, 0)
		, _objType(NULL)
	{
	}

	TileType::TileType(const TileType& other, const osg::CopyOp& copyop)
		: Object(other,copyop)
		, id(other.id)
		, index(other.index)
		, objType(other.objType)
		, flags(other.flags)
		, blockedArea(other.blockedArea)
		, name(other.name)
		, desc(other.desc)
		, appearance(copyop(other.appearance))
		, _objType(NULL)
	{
	}


	TileType::~TileType()
	{
	}

	void TileType::init(ObjectTypeMap* otm, TileTypeMap* ttm){
		_objType = otm->lookup(objType);
	}


	REG_OBJ_WRAPPER(game, TileType, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(index, 0);
		ADD_STRING_SERIALIZER(objType, "");
		ADD_INT_SERIALIZER(flags, 0);
		ADD_VEC2I_SERIALIZER(blockedArea, osg::Vec2i(-1, 0));
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_OBJECT_SERIALIZER(appearance, osg::Node, NULL);
	}

}
