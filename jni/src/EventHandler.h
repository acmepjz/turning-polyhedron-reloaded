#pragma once

#include "EventAction.h"
#include "util_object.h"

#include <vector>
#include <string>
#include <map>

class XMLNode;

namespace game {

	class Level;
	class MapData;
	class Polyhedron;

	class EventDescription;

	class EventHandler : public osg::Object {
	public:
		enum Type {
			INVALID = -1,
			ON_ENTER,
			ON_LEAVE,
			ON_MOVE_ENTER,
			ON_MOVE_LEAVE,
			ON_EVENT,
			ON_HIT_TEST,
			TYPE_MAX,
		};
	protected:
		virtual ~EventHandler();
	public:
		META_Object(game, EventHandler);

		EventHandler();
		EventHandler(const EventHandler& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool load(const XMLNode* node, int _type); //!< experimental

		static int convertToEventType(const std::string& name); //!< convert a node name to \ref Type.
		static const char* convertToEventName(int type); //!< get the node name of \ref Type.

		void processEvent(Level* parent, EventDescription* evt);
		
		UTIL_ADD_BYVAL_GETTER_SETTER(int, type);
		UTIL_ADD_BYREF_GETTER_SETTER(util::StringStringMap, conditions);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<EventAction> >, actions);

	public:
		int type; //!< the \ref Type
		util::StringStringMap conditions; //!< conditions
		std::vector<osg::ref_ptr<EventAction> > actions; //!< actions
	};

}



