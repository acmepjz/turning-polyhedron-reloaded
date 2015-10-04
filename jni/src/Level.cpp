#include "Level.h"
#include "util_err.h"
#include <osg/Group>
#include <osg/MatrixTransform>
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
			UTIL_ERR "object doesn't have id" << std::endl;
			return false;
		}

		MapDataMap::iterator it = maps.find(obj->id);
		if (it != maps.end()) {
			UTIL_WARN "object id '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
		}

		maps[obj->id] = obj;
		return true;
	}

	bool Level::addPolyhedron(Polyhedron* obj){
		if (!obj) return false;

		if (!obj->id.empty()) {
			PolyhedronMap::iterator it = _polyhedra.find(obj->id);
			if (it != _polyhedra.end()) {
				UTIL_WARN "object id '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
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
			(*it)->updateTransform();
			gp->addChild((*it)->_trans);
		}
		_appearance = gp;
	}

	void Level::initMaps(){
		_polyhedra.clear();
		for (Polyhedra::iterator it = polyhedra.begin(); it != polyhedra.end(); ++it) {
			if (!(*it)->id.empty()) {
				_polyhedra[(*it)->id] = *it;
			}
		}

		for (MapDataMap::iterator it = maps.begin(); it != maps.end(); ++it) {
			it->second->init(this);
		}
		for (Polyhedra::iterator it = polyhedra.begin(); it != polyhedra.end(); ++it) {
			(*it)->init(this);
		}
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
