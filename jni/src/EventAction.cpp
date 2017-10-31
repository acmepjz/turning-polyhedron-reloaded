#include "EventAction.h"
#include "EventHandler.h"
#include "Level.h"
#include "MapData.h"
#include "Polyhedron.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>
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
	"teleport",
	"cancel"
};

// NOTE: the items in each array should be sorted

static const char* actionArgRaiseEvent[] = { "target", "type", NULL };
static const char* actionArgRemoveObject[] = { "target", "type", NULL };
static const char* actionArgConvertTo[] = { "target", "value", NULL };
static const char* actionArgCheckpoint[] = { NULL };
static const char* actionArgMovePolyhedron[] = { NULL }; // TODO:
static const char* actionArgTeleport[] = { "dest", "flags", "hide", "size", "src", NULL };
static const char* actionArgCancel[] = { NULL };

static const char** actionArguments[game::EventAction::TYPE_MAX] =
{
	actionArgRaiseEvent,
	actionArgRemoveObject,
	actionArgConvertTo,
	actionArgCheckpoint,
	actionArgMovePolyhedron,
	actionArgTeleport,
	actionArgCancel,
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

		// report unknown arguments
		util::StringStringMap::const_iterator first1 = arguments.begin(), last1 = arguments.end();
		const char **first2 = actionArguments[type];

		while (first1 != last1 && *first2) {
			// debug
			assert(first2[1] == NULL || strcmp(first2[0], first2[1]) < 0);

			if (first1->first < *first2) {
				UTIL_WARN "unknown argument '" << first1->first << "' of action '" << convertToActionName(type) << "'" << std::endl;
				++first1;
			} else if (*first2 < first1->first) {
				++first2;
			} else {
				++first1; ++first2;
			}
		}

		while (first1 != last1) {
			UTIL_WARN "unknown argument '" << first1->first << "' of action '" << convertToActionName(type) << "'" << std::endl;
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

	static void findTargets(Level* parent, EventDescription* evt, const std::string& _targets,
		std::vector<Polyhedron::HitTestResult::Position>* retTile,
		std::vector<Polyhedron*>* retPolyhedron) {
		osgDB::StringList targets;
		osgDB::split(osgDB::trimEnclosingSpaces(_targets), targets);

		for (size_t i = 0; i < targets.size(); i++) {
			const std::string& target = targets[i];

			if (target.empty()) continue;
			if (target == "all") {
				if (retTile) {
					for (Level::MapDataMap::iterator it = parent->maps.begin(); it != parent->maps.end(); ++it) {
						MapData *map = it->second;

						const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);

						for (int z = sz; z < ez; z++) {
							for (int y = sy; y < ey; y++) {
								for (int x = sx; x < ex; x++) {
									Polyhedron::HitTestResult::Position p;
									p._map = map;
									p.position.set(x, y, z);
									retTile->push_back(p);
								}
							}
						}
					}
				} else {
					UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
				}
			} else if (target == "this") {
				if (retTile) {
					Polyhedron::HitTestResult::Position p;
					p._map = evt->_map;
					p.position = evt->position;
					retTile->push_back(p);
				} else {
					UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
				}
			} else if (target == "polyhedron") {
				if (retPolyhedron) {
					if (evt->polyhedron) {
						retPolyhedron->push_back(evt->polyhedron);
					} else {
						UTIL_ERR "Polyhedron is NULL when requiring it" << std::endl;
					}
				} else {
					UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
				}
			} else if (target == "allPolyhedron") {
				if (retPolyhedron) {
					for (Level::Polyhedra::iterator it = parent->polyhedra.begin(); it != parent->polyhedra.end(); ++it) {
						retPolyhedron->push_back(*it);
					}
				} else {
					UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
				}
			} else {
				size_t lps = target.find_first_of('.');
				if (lps != std::string::npos) {
					// id.tag
					if (retTile) {
						Level::MapDataMap::iterator it = parent->maps.find(osgDB::trimEnclosingSpaces(target.substr(0, lps)));
						if (it != parent->maps.end()) {
							std::vector<osg::Vec3i> ppp;
							it->second->findAllTags(osgDB::trimEnclosingSpaces(target.substr(lps + 1)), ppp);
							for (size_t j = 0; j < ppp.size(); j++) {
								Polyhedron::HitTestResult::Position p;
								p._map = it->second;
								p.position = ppp[j];
								retTile->push_back(p);
							}
						}
					} else {
						UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
					}
				} else if ((lps = target.find_first_of('(')) != std::string::npos) {
					// id(coordinate)
					if (retTile) {
						Level::MapDataMap::iterator it = parent->maps.find(osgDB::trimEnclosingSpaces(target.substr(0, lps)));
						if (it != parent->maps.end()) {
							size_t lpe = target.find_first_of(')', lps);
							Polyhedron::HitTestResult::Position p;
							p._map = it->second;
							p.position = util::getAttrFromStringOsgVec(target.substr(lps + 1, lpe == std::string::npos ? lpe : lpe - lps - 1), osg::Vec3i());
							retTile->push_back(p);
						}
					} else {
						UTIL_WARN "Invalid use of '" << target << "'" << std::endl;
					}
				} else {
					// tag
					if (retTile) {
						for (Level::MapDataMap::iterator it = parent->maps.begin(); it != parent->maps.end(); ++it) {
							std::vector<osg::Vec3i> ppp;
							it->second->findAllTags(target, ppp);
							for (size_t j = 0; j < ppp.size(); j++) {
								Polyhedron::HitTestResult::Position p;
								p._map = it->second;
								p.position = ppp[j];
								retTile->push_back(p);
							}
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
			findTargets(parent, evt, arguments["target"], &ps, NULL); // TODO: can send event to polyhedron?

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

			std::vector<Polyhedron::HitTestResult::Position> ps;
			std::vector<Polyhedron*> polys;
			findTargets(parent, evt, arguments["target"], &ps, &polys);

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
			findTargets(parent, evt, arguments["target"], &ps, NULL);

			for (size_t i = 0; i < ps.size(); i++) {
				ps[i]._map->substituteTile(parent, ps[i].position[0], ps[i].position[1], ps[i].position[2], newTileType.get());
			}
		}
			break;
		case CHECKPOINT:
		{
			TileType *tt = evt->_map->get(evt->position);
			if (tt && (tt->flags & TileType::TileFlags::CHECKPOINT) != 0) {
				parent->_checkpointObtained++;
			} else {
				UTIL_WARN "Invalid use of checkpoint event" << std::endl;
			}
		}
			break;
		case MOVE_POLYHEDRON:
			// TODO:
			UTIL_NOTICE "TODO: " << convertToActionName(type) << std::endl;
			break;
		case TELEPORT_POLYHEDRON:
		{
			// hide some polyhedra
			std::vector<Polyhedron*> polys;
			findTargets(parent, evt, arguments["hide"], NULL, &polys);

			for (size_t i = 0; i < polys.size(); i++) {
				polys[i]->onRemove(parent, "teleport");
			}

			// get src polyhedron
			std::string src = osgDB::trimEnclosingSpaces(arguments["src"]);
			Polyhedron *srcPoly = NULL;
			if (src.empty() || src == "this" || src == "polyhedron") {
				srcPoly = evt->polyhedron;
			} else {
				Level::PolyhedronMap::iterator it = parent->_polyhedra.find(src);
				if (it != parent->_polyhedra.end()) srcPoly = it->second;
			}

			if (!srcPoly) {
				UTIL_ERR "Can't find polyhedron '" << src << "'" << std::endl;
				break;
			}

			// get dest position
			std::string dest = osgDB::trimEnclosingSpaces(arguments["dest"]);
			PolyhedronPosition pp;
			if (!pp.load(dest, parent, NULL) || (pp.init(parent), !pp._map)) {
				UTIL_ERR "Can't find destination '" << dest << "'" << std::endl;
				break;
			}

			// get dest rotation
			int flags = atoi(arguments["flags"].c_str());
			if (flags == 0) {
				osg::Vec2i size = util::getAttrFromStringOsgVec(arguments["size"], osg::Vec2i(srcPoly->size[0], srcPoly->size[1]));
				int candidateFlags[6] =
				{
					PolyhedronPosition::ROT_XYZ,
					PolyhedronPosition::ROT_YZX,
					PolyhedronPosition::ROT_ZXY,
					PolyhedronPosition::ROT_XZY | PolyhedronPosition::UPPER_X,
					PolyhedronPosition::ROT_YXZ | PolyhedronPosition::UPPER_X,
					PolyhedronPosition::ROT_ZYX | PolyhedronPosition::UPPER_X,
				};
				PolyhedronPosition::Idx idx;
				for (int i = 0; i < 6; i++) {
					PolyhedronPosition::getCurrentPos(candidateFlags[i], srcPoly, idx);
					if (idx.size[0] == size[0] && idx.size[1] == size[1]) {
						flags = candidateFlags[i];
						break;
					}
				}
			}
			if (flags == 0) {
				UTIL_WARN "Can't determine polyhedron rotation to fit the specified size" << std::endl;
			} else {
				pp.flags = flags;
			}

			// TODO: animation & check stability reason
			srcPoly->pos = pp;
			srcPoly->flags |= Polyhedron::VISIBLE;
			srcPoly->updateVisible();
			srcPoly->updateTransform();
			if (!srcPoly->valid(parent)) {
				srcPoly->onRemove(parent, "fall");
			}
		}
			break;
		case CANCEL:
			parent->_cancel = true;
			break;
		}
	}

	REG_OBJ_WRAPPER(game, EventAction, "")
	{
		ADD_INT_SERIALIZER(type, -1);
		ADD_MAP_SERIALIZER(arguments, util::StringStringMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_STRING);
	}

}
