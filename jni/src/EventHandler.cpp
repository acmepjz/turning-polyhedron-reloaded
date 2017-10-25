#include "EventHandler.h"
#include "Polyhedron.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include <stdlib.h>
#include <math.h>

#include <osgDB/XmlParser>
#include <osgDB/ObjectWrapper>

static const char* eventNames[game::EventHandler::TYPE_MAX] = {
	"onEnter",
	"onLeave",
	"onMoveEnter",
	"onMoveLeave",
	"onEvent",
	"onHitTest",
};

class RationalOrFloat {
private:
	int numerator;
	int denominator; // should be >0, otherwise it is float
	float f;

	void simplify() {
		if (denominator <= 0) return;

		int a = numerator, b = denominator, c;
		if (a < 0) a = -a;
		while ((c = a%b) != 0) {
			a = b;
			b = c;
		}

		numerator /= b;
		denominator /= b;
	}
public:
	void setRational(int num, int den = 1) {
		if (den < 0) {
			num = -num;
			den = -den;
		} else if (den == 0) {
			UTIL_WARN "Division by zero" << std::endl;
			num = 0;
			den = 1;
		}
		numerator = num;
		denominator = den;
		f = 0.0f;
		simplify();
	}

	void setFloat(float _f) {
		numerator = 0;
		denominator = -1;
		f = _f;
	}

	void setString(const std::string& s) {
		if (s.find_first_of(".eE") != std::string::npos) {
			numerator = 0;
			denominator = -1;
			f = atof(s.c_str());
		} else {
			size_t lps = s.find_first_of("/\\");
			if (lps != std::string::npos) {
				int num = atoi(s.c_str());
				int den = atoi(s.c_str() + (lps + 1));
				setRational(num, den);
			} else {
				numerator = atoi(s.c_str());
				denominator = 1;
				f = 0.0f;
			}
		}
	}

	explicit RationalOrFloat(int num, int den = 1) {
		setRational(num, den);
	}

	explicit RationalOrFloat(float _f) {
		setFloat(_f);
	}

	explicit RationalOrFloat(const std::string& s) {
		setString(s);
	}

	bool operator==(const RationalOrFloat& other) const {
		if (denominator <= 0) {
			if (other.denominator <= 0) return f == other.f;
			return f == (float)other.numerator / other.denominator;
		} else {
			if (other.denominator <= 0) return (float)numerator / denominator == other.f;
			return numerator == other.numerator && denominator == other.denominator;
		}
	}

	bool operator!=(const RationalOrFloat& other) const {
		return !(*this == other);
	}

	bool operator<=(const RationalOrFloat& other) const {
		if (denominator <= 0) {
			if (other.denominator <= 0) return f <= other.f;
			return f <= (float)other.numerator / other.denominator;
		} else {
			if (other.denominator <= 0) return (float)numerator / denominator <= other.f;
			return numerator * other.denominator <= other.numerator * denominator;
		}
	}

	bool operator>=(const RationalOrFloat& other) const {
		return !(*this < other);
	}

	bool operator<(const RationalOrFloat& other) const {
		if (denominator <= 0) {
			if (other.denominator <= 0) return f < other.f;
			return f < (float)other.numerator / other.denominator;
		} else {
			if (other.denominator <= 0) return (float)numerator / denominator < other.f;
			return numerator * other.denominator < other.numerator * denominator;
		}
	}

	bool operator>(const RationalOrFloat& other) const {
		return !(*this <= other);
	}
};

static bool checkCondition(const RationalOrFloat& n, const std::string& cond) {
	size_t lps = cond.find_first_of('!');
	if (lps != std::string::npos) {
		// !=
		return n != RationalOrFloat(cond.substr(lps + 1));
	}

	lps = cond.find_first_of('~');
	if (lps == std::string::npos) {
		// ==
		return n == RationalOrFloat(cond);
	}

	std::string lower = osgDB::trimEnclosingSpaces(cond.substr(0, lps));
	std::string upper = osgDB::trimEnclosingSpaces(cond.substr(lps + 1));

	if (!lower.empty() && n < RationalOrFloat(lower)) return false;
	if (!upper.empty() && n > RationalOrFloat(upper)) return false;
	return true;
}

static bool checkCondition(const std::string& s, const std::string& cond) {
	osgDB::StringList list;
	bool invert = false;
	bool ret = false;

	size_t lps = cond.find_first_of('!');
	if (lps != std::string::npos) {
		invert = true;
		osgDB::split(cond.substr(lps + 1), list, '|');
	} else {
		osgDB::split(cond, list, '|');
	}

	for (size_t i = 0; i < list.size(); i++) {
		if (s == osgDB::trimEnclosingSpaces(list[i])) {
			ret = true;
			break;
		}
	}

	return invert ? !ret : ret;
}

static bool checkCondition(bool b, const std::string& cond) {
	if (b) {
		return cond == "true" || atoi(cond.c_str()) != 0;
	} else {
		return cond == "false" || atoi(cond.c_str()) == 0;
	}
}

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

	EventHandler::~EventHandler() {

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

	void EventHandler::processEvent(Level* parent, EventDescription* evt) {
		if (type != evt->type) return;

		// debug only
		UTIL_INFO "event=" << convertToEventName(type)
			<< ", map=0x" << std::hex << intptr_t(evt->_map) << std::dec
			<< ", pos=(" << evt->position[0] << "," << evt->position[1] << "," << evt->position[2] << ")" << std::endl;

		// check conditions

#define CHECK_CONDITION(NAME, FIRST) \
	} else if (it->first == NAME) { \
		if (checkCondition(FIRST, it->second)) continue

		for (util::StringStringMap::const_iterator it = conditions.begin(); it != conditions.end(); ++it) {
			if (false) {
				CHECK_CONDITION("onGroundCount", RationalOrFloat(evt->onGroundCount));
				CHECK_CONDITION("weight", RationalOrFloat(evt->weight));
				CHECK_CONDITION("pressure", RationalOrFloat(evt->weight, evt->onGroundCount));
				CHECK_CONDITION("onDifferentType", evt->tileTypeCount > 1); // backward compatibility
				CHECK_CONDITION("onSameType", evt->tileTypeCount <= 1); // backward compatibility
				CHECK_CONDITION("tileTypeCount", RationalOrFloat(evt->tileTypeCount));
				CHECK_CONDITION("objectTypeCount", RationalOrFloat(evt->objectTypeCount));
				CHECK_CONDITION("type", evt->eventType);
			} else {
				int flags = 0;
				for (int i = 0; Polyhedron::polyhedronFlagsAndNames[i].name; i++) {
					if (it->first == Polyhedron::polyhedronFlagsAndNames[i].name) {
						flags = Polyhedron::polyhedronFlagsAndNames[i].flags;
						break;
					}
				}

				if (flags) {
					if (evt->polyhedron) {
						if (checkCondition((evt->polyhedron->flags & flags) != 0, it->second)) continue;
					} else {
						UTIL_ERR "Polyhedron is NULL when checking condition: " << it->first << std::endl;
						continue;
					}
				} else {
					UTIL_WARN "Unknown condition: " << it->first << std::endl;
					continue;
				}
			}

			// condition not satisfied
			return;
		}

#undef CHECK_CONDITION

		// condition satisfied

		// debug only
		UTIL_INFO << "Condition satisfied" << std::endl;

		// raise events
		for (size_t i = 0; i < actions.size(); i++) {
			actions[i]->processEvent(parent, evt);
		}
	}

	REG_OBJ_WRAPPER(game, EventHandler, "")
	{
		ADD_INT_SERIALIZER(type, -1);
		ADD_MAP_SERIALIZER(conditions, util::StringStringMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_STRING);
		ADD_VECTOR_SERIALIZER(actions, std::vector<osg::ref_ptr<EventAction> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}

}
