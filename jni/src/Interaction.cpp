#include "Interaction.h"
#include <osgDB/ObjectWrapper>

namespace game {

	Interaction::Interaction()
	{

	}

	Interaction::Interaction(const Interaction& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
	{

	}

	Interaction::~Interaction(){

	}

	REG_OBJ_WRAPPER(game, Interaction, "")
	{

	}

}
