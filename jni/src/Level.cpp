#include "Level.h"
#include "util_err.h"
#include "XMLReaderWriter.h"
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>

namespace game {

	Level::Level()
		: _currentPolyhedron(-1)
		, _isAnimating(true)
	{
	}

	Level::Level(const Level& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, solution(other.solution)
		, tileTypeMap(util::copyObj(other.tileTypeMap.get(), copyop)) //always deep copy
		, objectTypeMap(util::copyObj(other.objectTypeMap.get(), copyop)) //always deep copy
		, _currentPolyhedron(other._currentPolyhedron)
		, _isAnimating(true)
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

	void Level::createInstance(bool isEditMode){
		osg::ref_ptr<osg::Group> gp = new osg::Group;
		for (MapDataMap::iterator it = maps.begin(); it != maps.end(); ++it) {
			it->second->createInstance(isEditMode);
			gp->addChild(it->second->_appearance);
		}
		for (int i = 0, m = polyhedra.size(); i < m; i++) {
			Polyhedron *poly = polyhedra[i].get();
			poly->createInstance(isEditMode);
			if (i == _currentPolyhedron) poly->setSelected(true);
			poly->updateTransform();
			gp->addChild(poly->_trans);
		}
		_appearance = gp;
	}

	void Level::initMaps(){
		_checkpointCount = 0;
		_mainPolyhedronCount = 0;
		_isGameOver = false;

		_polyhedra.clear();
		for (Polyhedra::iterator it = polyhedra.begin(); it != polyhedra.end(); ++it) {
			if (!(*it)->id.empty()) {
				_polyhedra[(*it)->id] = *it;
			}
		}

		for (MapDataMap::iterator it = maps.begin(); it != maps.end(); ++it) {
			it->second->init(this);
			_checkpointCount += it->second->_checkpointCount;
		}

		for (Polyhedra::iterator it = polyhedra.begin(); it != polyhedra.end(); ++it) {
			(*it)->init(this);
			if ((*it)->flags & Polyhedron::PolyhedronFlags::MAIN) {
				_mainPolyhedronCount++;
			}
		}

		switchToFirstPolyhedron();
	}

	void Level::switchToNextPolyhedron(int prev){
		Polyhedron *current = getSelectedPolyhedron();
		if (current) current->setSelected(false);

		int i = 0, m = polyhedra.size();
		if (prev < 0 || prev >= m) prev = -1;

		for (; i < m; i++) {
			prev++;
			if (prev >= m) prev = 0;

			Polyhedron *poly = polyhedra[prev].get();
			if (poly->controller == Polyhedron::PLAYER && (poly->flags & Polyhedron::VISIBLE)) {
				poly->setSelected(true);
				_currentPolyhedron = prev;
				return;
			}
		}

		//can't find available polyhedron
		_currentPolyhedron = -1;
	}

	bool Level::load(const XMLNode* node) {
		//load subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "name") {
				name = subnode->contents;
			} else if (subnode->name == "solution") {
				solution = subnode->contents;
			} else if (subnode->name == "mapData") {
				osg::ref_ptr<MapData> md = new MapData;
				if (md->load(subnode, this)) {
					addMapData(md.get());
				} else {
					UTIL_WARN "failed to load map data" << std::endl;
				}
			} else if (subnode->name == "objectType") {
				getOrCreateObjectTypeMap()->loadObjectType(subnode);
			} else if (subnode->name == "tileType") {
				getOrCreateTileTypeMap()->loadTileType(subnode);
			} else if (subnode->name == "tileMapping") {
				getOrCreateTileTypeMap()->loadTileMapping(subnode);
			} else if (subnode->name == "polyhedron") {
				osg::ref_ptr<Polyhedron> poly = new Polyhedron;
				if (poly->load(subnode, this, NULL)) {
					addPolyhedron(poly.get());
				} else {
					UTIL_WARN "failed to load polyhedron" << std::endl;
				}
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	bool Level::update() {
		_isAnimating = false;
		for (int i = 0, m = polyhedra.size(); i < m; i++) {
			if (polyhedra[i]->update(this)) _isAnimating = true;
		}
		return _isAnimating;
	}

	void Level::processEvent() {
		while (!_eventQueue.empty()) {
			std::vector<osg::ref_ptr<EventDescription> > q;
			std::swap(_eventQueue, q);

			for (size_t i = 0; i < q.size(); i++) {
				EventDescription *evt = q[i];
				evt->_map->processEvent(this, evt);
			}
		}
	}

	REG_OBJ_WRAPPER(game, Level, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(solution, "");
		ADD_OBJECT_SERIALIZER(objectTypeMap, ObjectTypeMap, NULL);
		ADD_OBJECT_SERIALIZER(tileTypeMap, TileTypeMap, NULL);
		ADD_MAP_SERIALIZER(maps, Level::MapDataMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_VECTOR_SERIALIZER(polyhedra, Level::Polyhedra, osgDB::BaseSerializer::RW_OBJECT, -1);
	}
}
