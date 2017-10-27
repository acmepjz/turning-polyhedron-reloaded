#include "EventAction.h"
#include "EventHandler.h"
#include "Level.h"
#include "MapData.h"
#include "Polyhedron.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include <assert.h>
#include <string.h>

#include <osgDB/XmlParser>
#include <osgDB/ObjectWrapper>

#define SX(X) s##X = map->lbound.X()
#define EX(X) e##X = map->lbound.X() + map->size.X()

static const char* actionNames[game::EventAction::TYPE_MAX] =
{
	"raiseEvent",
	"remove",
	"convertTo",
	"checkpoint",
	"move",
};

// NOTE: the items in each array should be sorted

static const char* actionArgRaiseEvent[] = { "target", "type", NULL };
static const char* actionArgRemoveObject[] = { "target", "type", NULL };
static const char* actionArgConvertTo[] = { "target", "value", NULL };
static const char* actionArgCheckpoint[] = { NULL };
static const char* actionArgMovePolyhedron[] = { NULL }; // TODO:

static const char** actionArguments[game::EventAction::TYPE_MAX] =
{
	actionArgRaiseEvent,
	actionArgRemoveObject,
	actionArgConvertTo,
	actionArgCheckpoint,
	actionArgMovePolyhedron,
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

		// report unused arguments
		util::StringStringMap::const_iterator first1 = arguments.begin(), last1 = arguments.end();
		const char **first2 = actionArguments[type];

		while (first1 != last1 && *first2) {
			// debug
			assert(first2[1] == NULL || strcmp(first2[0], first2[1]) < 0);

			if (first1->first < *first2) {
				UTIL_WARN "argument '" << first1->first << "' of action '" << convertToActionName(type) << "' is unused" << std::endl;
				++first1;
			} else if (*first2 < first1->first) {
				++first2;
			} else {
				++first1; ++first2;
			}
		}

		while (first1 != last1) {
			UTIL_WARN "argument '" << first1->first << "' of action '" << convertToActionName(type) << "' is unused" << std::endl;
			++first1;
		}

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

	static void findTargets(Level* parent, EventDescription* evt, const std::string& _targets, std::vector<Polyhedron::HitTestResult::Position>& ret, std::vector<Polyhedron*>* retPolyhedron) {
		osgDB::StringList targets;
		osgDB::split(osgDB::trimEnclosingSpaces(_targets), targets);

		for (size_t i = 0; i < targets.size(); i++) {
			const std::string& target = targets[i];

			if (target.empty()) continue;
			if (target == "all") {
				// all
				for (Level::MapDataMap::iterator it = parent->maps.begin(); it != parent->maps.end(); ++it) {
					MapData *map = it->second;

					const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);

					for (int z = sz; z < ez; z++) {
						for (int y = sy; y < ey; y++) {
							for (int x = sx; x < ex; x++) {
								Polyhedron::HitTestResult::Position p;
								p._map = map;
								p.position.set(x, y, z);
								ret.push_back(p);
							}
						}
					}
				}
			} else if (target == "this") {
				// this
				Polyhedron::HitTestResult::Position p;
				p._map = evt->_map;
				p.position = evt->position;
				ret.push_back(p);
			} else if (retPolyhedron && target == "polyhedron") {
				if (evt->polyhedron) {
					retPolyhedron->push_back(evt->polyhedron);
				} else {
					UTIL_ERR "Polyhedron is NULL when requiring it" << std::endl;
				}
			} else if (retPolyhedron && target == "allPolyhedron") {
				for (Level::Polyhedra::iterator it = parent->polyhedra.begin(); it != parent->polyhedra.end(); ++it) {
					retPolyhedron->push_back(*it);
				}
			} else {
				size_t lps = target.find_first_of('.');
				if (lps != std::string::npos) {
					// id.tag
					Level::MapDataMap::iterator it = parent->maps.find(osgDB::trimEnclosingSpaces(target.substr(0, lps)));
					if (it != parent->maps.end()) {
						std::vector<osg::Vec3i> ppp;
						it->second->findAllTags(osgDB::trimEnclosingSpaces(target.substr(lps + 1)), ppp);
						for (size_t j = 0; j < ppp.size(); j++) {
							Polyhedron::HitTestResult::Position p;
							p._map = it->second;
							p.position = ppp[j];
							ret.push_back(p);
						}
					}
				} else if ((lps = target.find_first_of('(')) != std::string::npos) {
					// id(coordinate)
					Level::MapDataMap::iterator it = parent->maps.find(osgDB::trimEnclosingSpaces(target.substr(0, lps)));
					if (it != parent->maps.end()) {
						size_t lpe = target.find_first_of(')', lps);
						Polyhedron::HitTestResult::Position p;
						p._map = it->second;
						p.position = util::getAttrFromStringOsgVec(target.substr(lps + 1, lpe == std::string::npos ? lpe : lpe - lps - 1), osg::Vec3i());
						ret.push_back(p);
					}
				} else {
					// tag
					for (Level::MapDataMap::iterator it = parent->maps.begin(); it != parent->maps.end(); ++it) {
						std::vector<osg::Vec3i> ppp;
						it->second->findAllTags(target, ppp);
						for (size_t j = 0; j < ppp.size(); j++) {
							Polyhedron::HitTestResult::Position p;
							p._map = it->second;
							p.position = ppp[j];
							ret.push_back(p);
						}
					}
					if (retPolyhedron) {
						for (Level::Polyhedra::iterator it = parent->polyhedra.begin(); it != parent->polyhedra.end(); ++it) {
							if ((*it)->id == target) retPolyhedron->push_back(*it);
						}
					}
				}
			}
		}
	}

	void EventAction::processEvent(Level* parent, EventDescription* evt) {
		switch (type) {
		case RAISE_EVENT:
		{
			std::string eventType = osgDB::trimEnclosingSpaces(arguments["type"]);

			std::vector<Polyhedron::HitTestResult::Position> ps;
			findTargets(parent, evt, arguments["target"], ps, NULL); // TODO: can send event to polyhedron?

			for (size_t i = 0; i < ps.size(); i++) {
				osg::ref_ptr<EventDescription> evt2 = new EventDescription(*evt);
				evt2->type = EventHandler::ON_EVENT;
				evt2->_map = ps[i]._map;
				evt2->position = ps[i].position;
				evt2->eventType = eventType;
				parent->addEvent(evt2);
			}
		}
			break;
		case REMOVE_OBJECT:
		{
			std::string type = osgDB::trimEnclosingSpaces(arguments["type"]);
			std::string target = osgDB::trimEnclosingSpaces(arguments["target"]);

			std::vector<Polyhedron::HitTestResult::Position> ps;
			std::vector<Polyhedron*> polys;
			findTargets(parent, evt, target, ps, &polys);

			for (size_t i = 0; i < ps.size(); i++) {
				// TODO: animation (now it is simply convertTo 0)
				ps[i]._map->substituteTile(parent, ps[i].position[0], ps[i].position[1], ps[i].position[2], NULL);
			}

			for (size_t i = 0; i < polys.size(); i++) {
				polys[i]->onRemove(parent, type);
			}
		}
			break;
		case CONVERT_TO:
		{
			osg::ref_ptr<TileType> newTileType = parent->getOrCreateTileTypeMap()->lookup(osgDB::trimEnclosingSpaces(arguments["value"]));

			std::vector<Polyhedron::HitTestResult::Position> ps;
			findTargets(parent, evt, arguments["target"], ps, NULL);

			for (size_t i = 0; i < ps.size(); i++) {
				ps[i]._map->substituteTile(parent, ps[i].position[0], ps[i].position[1], ps[i].position[2], newTileType.get());
			}
		}
			break;
		case CHECKPOINT:
		{
			TileType *tt = evt->_map->get(evt->position);
			if (tt && (tt->flags & TileType::TileFlags::CHECKPOINT) != 0) {
				parent->_checkpointCount--;
			} else {
				UTIL_WARN "Invalid use of checkpoint event" << std::endl;
			}
		}
			break;
		case MOVE_POLYHEDRON:
			// TODO:
			break;
		}
	}

	REG_OBJ_WRAPPER(game, EventAction, "")
	{
		ADD_INT_SERIALIZER(type, -1);
		ADD_MAP_SERIALIZER(arguments, util::StringStringMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_STRING);
	}

}
