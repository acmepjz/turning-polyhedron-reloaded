#include "Polyhedron.h"
#include "ObjectType.h"

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

}
