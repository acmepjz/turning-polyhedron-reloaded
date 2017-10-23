#pragma once

#include "util_object.h"

class XMLNode;

namespace game {

	class EventAction : public osg::Object {
	public:
		enum Type {
			INVALID = -1,
			RAISE_EVENT,
			REMOVE_OBJECT,
			CONVERT_TO,
			CHECKPOINT,
			MOVE_POLYHEDRON,
			TYPE_MAX,
		};
	protected:
		virtual ~EventAction();
	public:
		META_Object(game, EventAction);

		EventAction();
		EventAction(const EventAction& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool load(const XMLNode* node, int _type); //!< experimental

		static int convertToActionType(const std::string& name); //!< convert a node name to \ref Type.
		static const char* convertToActionName(int type); //!< get the node name of \ref Type.

		UTIL_ADD_BYVAL_GETTER_SETTER(int, type);
		UTIL_ADD_BYREF_GETTER_SETTER(util::StringStringMap, arguments);

	public:
		int type; //!< the \ref Type
		util::StringStringMap arguments; //!< arguments
	};

}



