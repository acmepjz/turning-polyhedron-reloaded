#include "EventHandler.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include <osgDB/ObjectWrapper>

static const char* eventNames[game::EventHandler::TYPE_MAX] = {
	"onEnter",
	"onLeave",
	"onMoveEnter",
	"onMoveLeave",
	"onEvent",
	"onHitTest",
};

namespace game {

	EventHandler::EventHandler()
		: type(INVALID)
	{

	}

	EventHandler::EventHandler(const EventHandler& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, type(other.type)
		, conditions(other.conditions)
	{
		util::copyVector(actions, other.actions, copyop, true);
	}

	EventHandler::~EventHandler(){

	}

	bool EventHandler::load(const XMLNode* node, int _type){
		type = _type;
		conditions = node->attributes;

		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode *subnode = node->subNodes[i];
			int _actionType = EventAction::convertToActionType(subnode->name);
			if (_actionType >= 0) {
				osg::ref_ptr<EventAction> action = new EventAction();
				if (action->load(subnode, _actionType)) {
					actions.push_back(action);
				}
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	int EventHandler::convertToEventType(const std::string& name) {
		for (int i = 0; i < TYPE_MAX; i++) {
			if (name == eventNames[i]) return i;
		}
		return INVALID;
	}

	const char* EventHandler::convertToEventName(int type) {
		if (type >= 0 && type < TYPE_MAX) return eventNames[type];
		return NULL;
	}

	REG_OBJ_WRAPPER(game, EventHandler, "")
	{
		ADD_INT_SERIALIZER(type, -1);
		ADD_MAP_SERIALIZER(conditions, util::StringStringMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_STRING);
		ADD_VECTOR_SERIALIZER(actions, std::vector<osg::ref_ptr<EventAction> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}

}
