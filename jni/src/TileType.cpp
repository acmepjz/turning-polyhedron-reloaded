#include "TileType.h"
#include "util_err.h"
#include <osg/Node>
#include <osg/Notify>
#include <stdlib.h>
#include <osgDB/ObjectWrapper>

namespace game {

	TileType::TileType()
		: index(0)
		, flags(0)
		, blockedArea(-1, 0)
		, _objType(NULL)
	{
	}

	TileType::TileType(const TileType& other, const osg::CopyOp& copyop)
		: Object(other,copyop)
		, id(other.id)
		, index(other.index)
		, objType(other.objType)
		, flags(other.flags)
		, blockedArea(other.blockedArea)
		, name(other.name)
		, desc(other.desc)
		, appearance(util::copyObj(other.appearance.get(), copyop))
		, _objType(NULL)
	{
	}


	TileType::~TileType()
	{
	}

	void TileType::init(ObjectTypeMap* otm, TileTypeMap* ttm){
		_objType = otm->lookup(objType);
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

#define MyClass MyClass_TileType
	REG_OBJ_WRAPPER(game, TileType, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(index, 0);
		ADD_STRING_SERIALIZER(objType, "");
		ADD_INT_SERIALIZER(flags, 0);
		ADD_VEC2I_SERIALIZER(blockedArea, osg::Vec2i(-1, 0));
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_OBJECT_SERIALIZER(appearance, gfx::Appearance, NULL);
	}
#undef MyClass
#define MyClass MyClass_TileTypeMap
	REG_OBJ_WRAPPER(game, TileTypeMap, "")
	{
		ADD_MAP_SERIALIZER(idMap, TileTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_MAP_SERIALIZER(indexMap, TileTypeMap::IndexMap, osgDB::BaseSerializer::RW_INT, osgDB::BaseSerializer::RW_OBJECT);
	}
#undef MyClass

}
