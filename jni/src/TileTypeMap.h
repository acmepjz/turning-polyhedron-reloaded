#pragma once

#include <osg/Node>
#include <string>
#include "TileType.h"
#include "util_object.h"

namespace game {

	class ObjectTypeMap;

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

		void init(ObjectTypeMap* otm);

	public:
		typedef std::map<std::string, osg::ref_ptr<TileType> > IdMap;
		IdMap idMap;
		typedef std::map<int, osg::ref_ptr<TileType> > IndexMap;
		IndexMap indexMap;

		UTIL_ADD_BYREF_GETTER_SETTER(IdMap, idMap);
		UTIL_ADD_BYREF_GETTER_SETTER(IndexMap, indexMap);
	};

}
