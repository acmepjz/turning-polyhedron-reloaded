#include "TileType.h"
#include "util.h"
#include <osg/Node>
#include <osg/Notify>
#include <stdlib.h>

namespace game {

	TileType::TileType()
		: index(0)
	{
	}

	TileType::TileType(const TileType& other, const osg::CopyOp& copyop)
		: Object(other,copyop)
		, id(other.id)
		, index(other.index)
		, objType(other.objType)
		, blockedArea(other.blockedArea)
		, name(other.name)
		, desc(other.desc)
		, appearance(copyop(other.appearance))
	{
	}


	TileType::~TileType()
	{
	}

	TileTypeMap::TileTypeMap(){

	}

	TileTypeMap::TileTypeMap(const TileTypeMap& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
	{
		util::copyMap(idMap, other.idMap, copyop);
		util::copyMap(indexMap, other.indexMap, copyop);
	}

	TileTypeMap::~TileTypeMap(){

	}

	TileType* TileTypeMap::lookup(const std::string& idOrIndex){
		if (idOrIndex.empty()) return NULL;

		bool isNumeric = false;
		size_t m = idOrIndex.size();
		size_t i = idOrIndex[0] == '-' ? 1 : 0;
		if (i < m) {
			isNumeric = true;
			for (; i < m; i++) {
				char c = idOrIndex[i];
				if (c<'0' || c>'9') {
					isNumeric = false;
					break;
				}
			}
		}

		if (isNumeric) {
			int index = atoi(idOrIndex.c_str());
			if (index) {
				std::map<int, osg::ref_ptr<TileType> >::iterator it = indexMap.find(index);
				if (it != indexMap.end()) return it->second.get();
				OSG_NOTICE << "[" __FUNCTION__ "] index " << index << " not found" << std::endl;
			}
			return NULL;
		}

		std::map<std::string, osg::ref_ptr<TileType> >::iterator it = idMap.find(idOrIndex);
		if (it != idMap.end()) return it->second.get();
		OSG_NOTICE << "[" __FUNCTION__ "] id '" << idOrIndex << "' not found" << std::endl;
		return NULL;
	}

	bool TileTypeMap::addTileMapping(const std::string& id, int index){
		if (index == 0) {
			OSG_NOTICE << "[" __FUNCTION__ "] index 0 invalid" << std::endl;
			return false;
		}

		std::map<std::string, osg::ref_ptr<TileType> >::iterator it = idMap.find(id);
		if (it == idMap.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] id '" << id << "' not found" << std::endl;
			return false;
		}

		std::map<int, osg::ref_ptr<TileType> >::iterator it2 = indexMap.find(index);
		if (it2 != indexMap.end()) {
			OSG_NOTICE << "[" __FUNCTION__ "] index " << index << " already defined: '" << it2->second->id
				<< "', redefine to '" << id << "'" << std::endl;
			return false;
		}

		indexMap[index] = it->second.get();

		return true;
	}

}
