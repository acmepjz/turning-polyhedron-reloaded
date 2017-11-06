#pragma once

#include <osg/Node>
#include <string>
#include <map>
#include "util_object.h"
#include "MapData.h"
#include "Polyhedron.h"
#include "TileType.h"
#include "ObjectType.h"
#include "EventDescription.h"

class XMLNode;

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
		void createInstance(bool isEditMode);

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

		void initObjectTypes(){
			if (objectTypeMap.valid()) objectTypeMap->init();
		}
		void initTileTypes(){
			if (tileTypeMap.valid()) tileTypeMap->init(objectTypeMap.get());
		}
		void initMaps();
		void init(){
			initObjectTypes();
			initTileTypes();
			initMaps();
		}

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `level`

		void switchToPolyhedron(int index);

		void switchToFirstPolyhedron(){
			switchToNextPolyhedron(-1);
		}
		void switchToNextPolyhedron(){
			switchToNextPolyhedron(_currentPolyhedron);
		}
		void switchToNextPolyhedron(int prev);

		bool isPolyhedronSelected() const {
			return _currentPolyhedron >= 0 && _currentPolyhedron < (int)polyhedra.size();
		}
		Polyhedron* getSelectedPolyhedron() {
			if (isPolyhedronSelected()) return polyhedra[_currentPolyhedron].get();
			return NULL;
		}

		bool update(); //!< update animation
		bool isAnimating() const {
			return _isAnimating;
		}

		void addEvent(EventDescription* _event) {
			_eventQueue.push_back(_event);
		}
		void addEvent(std::vector<osg::ref_ptr<EventDescription> >& _events) {
			_eventQueue.insert(_eventQueue.end(), _events.begin(), _events.end());
		}
		void processEvent();

	public:
		std::string name; //!< level name
		std::string solution; //!< solution include in level file, for reference only

		int checkpointRequired; //!< the number of checkpoints required to win (saved to level file), if <=0 it means all but excluding such number of checkpoints

		typedef std::map<std::string, osg::ref_ptr<MapData> > MapDataMap;
		MapDataMap maps; //!< map blocks

		typedef std::vector<osg::ref_ptr<Polyhedron> > Polyhedra;
		Polyhedra polyhedra; //!< polyhedra

		osg::ref_ptr<TileTypeMap> tileTypeMap; //!< tile type map used in this level
		osg::ref_ptr<ObjectTypeMap> objectTypeMap; //!< object type map used in this level

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, solution);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, checkpointRequired);
		UTIL_ADD_BYREF_GETTER_SETTER(MapDataMap, maps);
		UTIL_ADD_BYREF_GETTER_SETTER(Polyhedra, polyhedra);
		UTIL_ADD_OBJ_GETTER_SETTER(TileTypeMap, tileTypeMap);
		UTIL_ADD_OBJ_GETTER_SETTER(ObjectTypeMap, objectTypeMap);

	public:
		//the following properties don't save to file and is generated at runtime
		typedef std::map<std::string, Polyhedron* > PolyhedronMap;
		PolyhedronMap _polyhedra; //!< polyhedra with a valid id

		osg::ref_ptr<osg::Node> _appearance; //!< the appearance

		int _currentPolyhedron; //!< current polyhedron

		bool _isAnimating; //!< is animating, updated when \ref update() is called
		bool _isGameOver;
		bool _isTileDirty;

		std::vector<osg::ref_ptr<EventDescription> > _eventQueue;
		bool _cancel; //!< used when processing preMoveEnter event, etc.

		int _moves;

		int _checkpointCount; //!< the number of checkpoints (a constant which doesn't change during playing)
		int _checkpointObtained; //!< the number of checkpoints obtained

		/** get the checkpoint required (normalized) */
		int getCheckpointRequired() const {
			return (checkpointRequired <= 0 ? _checkpointCount : 0) + checkpointRequired;
		}

		/** get the checkpoint remaining to win */
		int getCheckpointRemaining() const {
			return getCheckpointRequired() - _checkpointObtained;
		}

		/** check if checkpoint is enough */
		bool isCheckpointEnough() const {
			return getCheckpointRemaining() <= 0;
		}

		int _mainPolyhedronCount; //!< the number of main polyhedron which are still in the map
	};

}
