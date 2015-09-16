#include "Level.h"
#include <osg/Group>
#include <osgDB/ObjectWrapper>

namespace game {

	Level::Level()
	{
	}

	Level::Level(const Level& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, solution(other.solution)
		, tileTypeMap(util::copyObj(other.tileTypeMap.get(), copyop)) //always deep copy
		, objectTypeMap(util::copyObj(other.objectTypeMap.get(), copyop)) //always deep copy
	{
		//following objects are always deep copy
		util::copyMap(maps, other.maps, copyop, true);
		util::copyVector(polyhedra, other.polyhedra, copyop, true);
	}

	Level::~Level()
	{
	}

	bool Level::addMapData(MapData* obj){
		if (!obj || obj->id.empty()) {
			OSG_NOTICE << "[" __FUNCTION__ "] object doesn't have id" << std::endl;
			return false;
		}

		MapDataMap::iterator it = maps.find(obj->id);
		if (it != maps.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] object id '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
		}

		maps[obj->id] = obj;
		return true;
	}

	bool Level::addPolyhedron(Polyhedron* obj){
		if (!obj) return false;

		if (!obj->id.empty()) {
			PolyhedronMap::iterator it = _polyhedra.find(obj->id);
			if (it != _polyhedra.end()) {
				OSG_NOTICE << "[" __FUNCTION__ "] object id '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
			}
			_polyhedra[obj->id] = obj;
		}

		polyhedra.push_back(obj);

		return true;
	}

	void Level::createInstance(){
		osg::ref_ptr<osg::Group> gp = new osg::Group;
		for (MapDataMap::iterator it = maps.begin(); it != maps.end(); ++it) {
			it->second->createInstance();
			gp->addChild(it->second->_appearance);
		}
		for (Polyhedra::iterator it = polyhedra.begin(); it != polyhedra.end(); ++it) {
			(*it)->createInstance();
			//TODO: transformation matrix
			gp->addChild((*it)->_appearance);
		}
		_appearance = gp;
	}

	REG_OBJ_WRAPPER(game, Level, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(solution, "");
		ADD_OBJECT_SERIALIZER(objectTypeMap, ObjectTypeMap, NULL);
		ADD_OBJECT_SERIALIZER(tileTypeMap, TileTypeMap, NULL);
		ADD_MAP_SERIALIZER(maps, Level::MapDataMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_VECTOR_SERIALIZER(polyhedra, Level::Polyhedra, osgDB::BaseSerializer::RW_OBJECT, 1);
	}
}
