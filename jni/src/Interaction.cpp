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

	bool Interaction::load(const std::string& data){
		//TODO: load interactions
		return true;
	}

	REG_OBJ_WRAPPER(game, Interaction, "")
	{

	}

}
