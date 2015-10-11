#include "LevelCollection.h"
#include "util_err.h"
#include "XMLReaderWriter.h"
#include <osgDB/ObjectWrapper>

namespace game {

	LevelCollection::LevelCollection()
	{
	}

	LevelCollection::LevelCollection(const LevelCollection& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, tileTypeMap(util::copyObj(other.tileTypeMap.get(), copyop)) //always deep copy
		, objectTypeMap(util::copyObj(other.objectTypeMap.get(), copyop)) //always deep copy
	{
		//following objects are always deep copy
		util::copyVector(levels, other.levels, copyop, true);
	}

	LevelCollection::~LevelCollection()
	{
	}

	bool LevelCollection::load(const XMLNode* node) {
		//load subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "name") {
				name = subnode->contents;
			} else if (subnode->name == "level") {
				osg::ref_ptr<Level> level = new Level;
				level->objectTypeMap = new ObjectTypeMap(*getOrCreateObjectTypeMap(), osg::CopyOp::SHALLOW_COPY);
				level->tileTypeMap = new TileTypeMap(*getOrCreateTileTypeMap(), osg::CopyOp::SHALLOW_COPY);

				if (level->load(subnode)) {
					levels.push_back(level);
				} else {
					UTIL_WARN "failed to load level" << std::endl;
				}
			} else if (subnode->name == "objectType") {
				getOrCreateObjectTypeMap()->loadObjectType(subnode);
			} else if (subnode->name == "tileType") {
				getOrCreateTileTypeMap()->loadTileType(subnode);
			} else if (subnode->name == "tileMapping") {
				getOrCreateTileTypeMap()->loadTileMapping(subnode);
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	osg::Object* LevelCollection::loadLevelOrCollection(const XMLNode* node, ObjectTypeMap* otm, TileTypeMap* ttm) {
		if (node->name == "levelCollection") {
			osg::ref_ptr<LevelCollection> lc = new LevelCollection;
			if (otm) {
				lc->objectTypeMap = new ObjectTypeMap(*otm, osg::CopyOp::SHALLOW_COPY);
			}
			if (ttm) {
				lc->tileTypeMap = new TileTypeMap(*ttm, osg::CopyOp::SHALLOW_COPY);
			}
			if (lc->load(node)) return lc.release();
			else {
				UTIL_WARN "failed to load level collection" << std::endl;
			}
		} else if (node->name == "level") {
			osg::ref_ptr<Level> level = new Level;
			if (otm) {
				level->objectTypeMap = new ObjectTypeMap(*otm, osg::CopyOp::SHALLOW_COPY);
			}
			if (ttm) {
				level->tileTypeMap = new TileTypeMap(*ttm, osg::CopyOp::SHALLOW_COPY);
			}
			if (level->load(node)) return level.release();
			else {
				UTIL_WARN "failed to load level" << std::endl;
			}
		} else {
			UTIL_WARN "unrecognized node name: " << node->name << std::endl;
		}

		return NULL;
	}


	REG_OBJ_WRAPPER(game, LevelCollection, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_OBJECT_SERIALIZER(tileTypeMap, TileTypeMap, NULL);
		ADD_OBJECT_SERIALIZER(objectTypeMap, ObjectTypeMap, NULL);
		ADD_VECTOR_SERIALIZER(levels, std::vector<osg::ref_ptr<Level> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}
}
