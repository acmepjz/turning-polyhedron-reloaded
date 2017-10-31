#pragma once

#include <osg/Referenced>
#include <osg/Vec3i>

#include <vector>
#include <string>
#include <map>

class XMLNode;

namespace game {

	class Level;
	class MapData;
	class Polyhedron;

	class EventDescription : public osg::Referenced {
	protected:
		~EventDescription();
	public:
		EventDescription();

		int type; //!< see \ref EventHandler::Type
		MapData *_map; //!< the map
		osg::Vec3i position; //!< the position
		int onGroundCount; //!< how many blocks of polyhedron are supported
		int weight; //!< the weight of polyhedron
		int tileTypeCount; //!< number of types of tiles which are supporting polyhedron
		int objectTypeCount; //!< number of object types of tiles which are supporting polyhedron
		Polyhedron *polyhedron; //!< the polyhedron, can be NULL
		std::string eventType; //!< only used when ON_EVENT
	};

}



