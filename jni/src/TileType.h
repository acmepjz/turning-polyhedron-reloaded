#pragma once

#include <osg/Node>
#include <osg/Vec2i>
#include <string>
#include "ObjectType.h"
#include "Appearance.h"
#include "EventHandler.h"
#include "util_object.h"

namespace game {

	class Level;
	class ObjectTypeMap;
	class TileTypeMap;

	/// Represents a tile type.

	class TileType :
		public osg::Object
	{
	public:
		/// tile flags
		enum TileFlags {
			SUPPORTER = 0x1, //!< It can support other polyhedra.
			TILT_SUPPORTER = 0x2, //!< It can support other tilting polyhedra.
			CHECKPOINT = 0x4, //!< Count this tile as a checkpoint.
			TARGET = 0x8, //!< Count this tile as a target, e.g. in Sokoban.
			EXIT = 0x10, //!< It is an exit block (won't check shape of polyhedron).
		};
	protected:
		virtual ~TileType();
	public:
		META_Object(game, TileType);

		TileType();
		TileType(const TileType& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		void init(ObjectTypeMap* otm, TileTypeMap* ttm);

		bool load(const XMLNode* node, gfx::AppearanceMap* _template); //!< load from XML node, assume the node is called `tileType`

		/** (test only) */
		osg::Node* getOrCreateInstance(int shape, bool isEditMode);

		void processEvent(Level* parent, EventDescription* evt) {
			for (size_t i = 0; i < events.size(); i++) {
				events[i]->processEvent(parent, evt);
			}
		}

	public:
		std::string id; //!< id, used to find this tile type
		int index; //!< the (permanent) index (optional), used to find this tile type, 0 = no index

		std::string objType; //!< the object type
		int flags; //!< flags. \sa TileFlags

		/** z coordinate of blocked hit test area. The format is (lower,upper).
		For example ground=(-1,0), wall=(-1,1), non-block=(0,0).
		\note Note that normal hit test area is fixed at z=0.
		*/
		osg::Vec2i blockedArea;

		std::string name; //!< the gettext'ed object name
		std::string desc; //!< the gettext'ed object description

		/** The appearance map.
		Some predefined appearance id:
		- "" showed in both game and edit (but only showed in edit mode if invisibleAtRuntime=true)
		- "game" only showed in game
		- "edit" only showed in edit
		*/
		gfx::AppearanceMap appearanceMap;

		std::vector<osg::ref_ptr<EventHandler> > events; //!< the events

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, index);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, flags);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec2i, blockedArea);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, desc);
		UTIL_ADD_BYREF_GETTER_SETTER(gfx::AppearanceMap, appearanceMap);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<EventHandler> >, events);

	public:
		//the following properties don't save to file and is generated at runtime
		ObjectType* _objType;
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
		int _lastShape; //!< shape (last time)
		bool _isEditMode; //!< is edit mode (last time)
	};

	/// A map used to look up tile type

	class TileTypeMap : public osg::Object {
	protected:
		virtual ~TileTypeMap();
	public:
		META_Object(game, TileTypeMap);

		TileTypeMap();
		TileTypeMap(const TileTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool add(TileType* obj); //!< Add an object to map (which must has a valid id, optional index).
		TileType* lookup(const std::string& idOrIndex); //!< Find an object. If id is empty or "0" then returns NULL.

		bool addTileMapping(const std::string& id, int index); //!< Add a (temporary) index to a tile type with specified id
		bool addTileMapping(TileType* tile, int index); //!< Add a (temporary) index to a tile type

		void init(ObjectTypeMap* otm);

		bool load(const XMLNode* node, gfx::AppearanceMap* _template); //!< load from XML node, assume the node is called `tileTypes`
		bool loadTileType(const XMLNode* node, gfx::AppearanceMap* _template); //!< load from XML node, assume the node is called `tileType`
		bool loadTileMapping(const XMLNode* node); //!< load from XML node, assume the node is called `tileMapping`

	public:
		typedef std::map<std::string, osg::ref_ptr<TileType> > IdMap;
		IdMap idMap;
		typedef std::map<int, osg::ref_ptr<TileType> > IndexMap;
		IndexMap indexMap;

		UTIL_ADD_BYREF_GETTER_SETTER(IdMap, idMap);
		UTIL_ADD_BYREF_GETTER_SETTER(IndexMap, indexMap);
	};

}
