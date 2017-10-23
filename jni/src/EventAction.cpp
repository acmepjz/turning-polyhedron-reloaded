#include "EventAction.h"
#include "XMLReaderWriter.h"

#include <osgDB/ObjectWrapper>

static const char* actionNames[game::EventAction::TYPE_MAX] = {
	"raiseEvent",
	"remove",
	"convertTo",
	"checkpoint",
	"move",
};

namespace game {

	EventAction::EventAction()
		: type(INVALID)
	{

	}

	EventAction::EventAction(const EventAction& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, type(other.type)
		, arguments(other.arguments)
	{

	}

	EventAction::~EventAction(){

	}

	bool EventAction::load(const XMLNode* node, int _type){
		type = _type;
		arguments = node->attributes;

		return true;
	}

	int EventAction::convertToActionType(const std::string& name) {
		for (int i = 0; i < TYPE_MAX; i++) {
			if (name == actionNames[i]) return i;
		}
		return INVALID;
	}

	const char* EventAction::convertToActionName(int type) {
		if (type >= 0 && type < TYPE_MAX) return actionNames[type];
		return NULL;
	}

	REG_OBJ_WRAPPER(game, EventAction, "")
	{
		ADD_INT_SERIALIZER(type, -1);
		ADD_MAP_SERIALIZER(arguments, util::StringStringMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_STRING);
	}

}
