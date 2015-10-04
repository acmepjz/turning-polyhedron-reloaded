#include "TileTypeMap.h"
#include "TileType.h"
#include "util.h"
#include "util_err.h"
#include <osgDB/ObjectWrapper>
#include <stdlib.h>

namespace game {

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
				UTIL_WARN "index " << index << " not found" << std::endl;
			}
			return NULL;
		}

		std::map<std::string, osg::ref_ptr<TileType> >::iterator it = idMap.find(idOrIndex);
		if (it != idMap.end()) return it->second.get();
		UTIL_WARN "id '" << idOrIndex << "' not found" << std::endl;
		return NULL;
	}

	bool TileTypeMap::add(TileType* obj){
		if (!obj || obj->id.empty()) {
			UTIL_WARN "object doesn't have id" << std::endl;
			return false;
		}

		IdMap::iterator it = idMap.find(obj->id);
		if (it != idMap.end()) {
			UTIL_WARN "object id '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
		}

		idMap[obj->id] = obj;

		if (obj->index) {
			IndexMap::iterator it = indexMap.find(obj->index);
			if (it != indexMap.end()) {
				UTIL_WARN "object index " << it->first << " already defined : '" << it->second->id
					<< "', redefine to '" << obj->id << "'" << std::endl;
			}

			indexMap[obj->index] = obj;
		}

		return true;
	}

	bool TileTypeMap::addTileMapping(const std::string& id, int index){
		if (index == 0) {
			UTIL_WARN "index 0 invalid" << std::endl;
			return false;
		}

		std::map<std::string, osg::ref_ptr<TileType> >::iterator it = idMap.find(id);
		if (it == idMap.end()) {
			UTIL_WARN "id '" << id << "' not found" << std::endl;
			return false;
		}

		std::map<int, osg::ref_ptr<TileType> >::iterator it2 = indexMap.find(index);
		if (it2 != indexMap.end()) {
			UTIL_WARN "index " << index << " already defined : '" << it2->second->id
				<< "', redefine to '" << id << "'" << std::endl;
		}

		indexMap[index] = it->second.get();

		return true;
	}

	void TileTypeMap::init(ObjectTypeMap* otm){
		for (IdMap::iterator it = idMap.begin(); it != idMap.end(); ++it) {
			it->second->init(otm, this);
		}
	}

	REG_OBJ_WRAPPER(game, TileTypeMap, "")
	{
		ADD_MAP_SERIALIZER(idMap, TileTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_MAP_SERIALIZER(indexMap, TileTypeMap::IndexMap, osgDB::BaseSerializer::RW_INT, osgDB::BaseSerializer::RW_OBJECT);
	}

}
