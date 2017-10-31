#include "EventDescription.h"
#include "EventHandler.h"

namespace game {

	EventDescription::EventDescription()
		: type(EventHandler::INVALID)
		, _map(NULL)
		, onGroundCount(0)
		, weight(0)
		, tileTypeCount(0)
		, objectTypeCount(0)
		, polyhedron(NULL)
	{

	}

	EventDescription::~EventDescription() {

	}

}
