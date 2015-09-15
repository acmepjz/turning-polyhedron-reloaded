#pragma once

#include <osg/Object>
#include <osg/Node>
#include <string>
#include <map>
#include "util.h"
#include "MapData.h"
#include "Polyhedron.h"
#include "TileTypeMap.h"
#include "ObjectTypeMap.h"

namespace game {

	/// Represents a level.

	class Level :
		public osg::Object
	{
	protected:
		virtual ~Level();
	public:
		META_Object(game, Level);

		Level();
		Level(const Level& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		///test only
		osg::Node* createInstance();

		bool addMapData(MapData* obj); //!< Add a map (which must has a valid id).
		bool addPolyhedron(Polyhedron* obj); //!< Add a polyhedron (possibly no id).

		TileTypeMap* getOrCreateTileTypeMap(){
			if (!tileTypeMap.valid()) tileTypeMap = new TileTypeMap;
			return tileTypeMap.get();
		}
		ObjectTypeMap* getOrCreateObjectTypeMap(){
			if (!objectTypeMap.valid()) objectTypeMap = new ObjectTypeMap;
			return objectTypeMap.get();
		}

	public:
		std::string name; //!< level name
		std::string solution; //!< solution include in level file, for reference only

		typedef std::map<std::string, osg::ref_ptr<MapData> > MapDataMap;
		MapDataMap maps; //!< map blocks

		std::vector<osg::ref_ptr<Polyhedron> > polyhedra; //!< polyhedra

		osg::ref_ptr<TileTypeMap> tileTypeMap; //!< tile type map used in this level
		osg::ref_ptr<ObjectTypeMap> objectTypeMap; //!< object type map used in this level

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, solution);
		UTIL_ADD_BYREF_GETTER_SETTER(MapDataMap, maps);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<Polyhedron> >, polyhedra);
		UTIL_ADD_OBJ_GETTER_SETTER(TileTypeMap, tileTypeMap);
		UTIL_ADD_OBJ_GETTER_SETTER(ObjectTypeMap, objectTypeMap);

	public:
		//the following properties don't save to file and is generated at runtime
		typedef std::map<std::string, Polyhedron* > PolyhedronMap;
		PolyhedronMap _polyhedra; //!< polyhedra with a valid id
	};

}
