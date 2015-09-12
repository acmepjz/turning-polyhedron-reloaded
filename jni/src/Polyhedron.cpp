#include "Polyhedron.h"
#include "ObjectType.h"
#include <osgDB/ObjectWrapper>

namespace game {

	Polyhedron::Polyhedron()
		: flags(0)
	{
	}

	Polyhedron::Polyhedron(const Polyhedron& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, objType(other.objType)
		, flags(other.flags)
		, pos(other.pos)
		, appearance(copyop(other.appearance))
	{

	}

	Polyhedron::~Polyhedron()
	{
	}

	REG_OBJ_WRAPPER(game, Polyhedron, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_STRING_SERIALIZER(objType, "");
		ADD_INT_SERIALIZER(flags, 0);
		ADD_REF_ANY_SERIALIZER(pos, MapPosition, MapPosition());
		ADD_OBJECT_SERIALIZER(appearance, osg::Node, NULL);
	}

}
