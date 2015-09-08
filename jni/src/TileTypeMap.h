#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>
#include "TileType.h"

namespace game {

	/// A map used to look up tile type

	class TileTypeMap : public osg::Object {
	protected:
		virtual ~TileTypeMap();
	public:
		META_Object(game, TileTypeMap);

		TileTypeMap();
		TileTypeMap(const TileTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		TileType* lookup(const std::string& idOrIndex);

		//! add a (temporary) index to a tile type with specified id
		bool addTileMapping(const std::string& id, int index);

		typedef std::map<std::string, osg::ref_ptr<TileType> > IdMap;
		IdMap idMap;
		typedef std::map<int, osg::ref_ptr<TileType> > IndexMap;
		IndexMap indexMap;

		UTIL_ADD_BYREF_GETTER_SETTER(IdMap, idMap);
		UTIL_ADD_BYREF_GETTER_SETTER(IndexMap, indexMap);
	};

}
