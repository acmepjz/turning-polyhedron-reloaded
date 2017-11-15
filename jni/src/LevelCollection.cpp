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
		, appearanceMap(util::copyObj(other.appearanceMap.get(), copyop))
	{
		//following objects are always deep copy
		util::copyVector(levels, other.levels, copyop, true);

		// set parent
		for (size_t i = 0; i < levels.size(); i++) {
			levels[i]->getOrCreateObjectTypeMap()->parent = getOrCreateObjectTypeMap();
			levels[i]->getOrCreateTileTypeMap()->parent = getOrCreateTileTypeMap();
			levels[i]->getOrCreateAppearanceMap()->parent = getOrCreateAppearanceMap();
		}
	}

	LevelCollection::~LevelCollection()
	{
	}

	bool LevelCollection::load(const XMLNode* node) {
		ObjectTypeMap* otm = getOrCreateObjectTypeMap();
		TileTypeMap* ttm = getOrCreateTileTypeMap();
		gfx::AppearanceMap *am = getOrCreateAppearanceMap();

		//load subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "name") {
				name = subnode->contents;
			} else if (subnode->name == "level") {
				osg::ref_ptr<Level> level = new Level;
				level->getOrCreateObjectTypeMap()->parent = otm;
				level->getOrCreateTileTypeMap()->parent = ttm;
				level->getOrCreateAppearanceMap()->parent = am;

				if (level->load(subnode)) {
					levels.push_back(level);
				} else {
					UTIL_WARN "failed to load level" << std::endl;
				}
			} else if (subnode->name == "objectType") {
				otm->loadObjectType(subnode);
			} else if (subnode->name == "tileType") {
				ttm->loadTileType(subnode, am);
			} else if (subnode->name == "appearances") {
				am->load(subnode, am, std::string(), osg::Vec3(1, 1, 1));
			} else if (subnode->name == "tileMapping") {
				ttm->loadTileMapping(subnode);
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	osg::Object* LevelCollection::loadLevelOrCollection(const XMLNode* node, ObjectTypeMap* otm, TileTypeMap* ttm, gfx::AppearanceMap* am) {
		if (node->name == "levelCollection") {
			osg::ref_ptr<LevelCollection> lc = new LevelCollection;
			lc->getOrCreateObjectTypeMap()->parent = otm;
			lc->getOrCreateTileTypeMap()->parent = ttm;
			lc->getOrCreateAppearanceMap()->parent = am;
			if (lc->load(node)) return lc.release();
			else {
				UTIL_WARN "failed to load level collection" << std::endl;
			}
		} else if (node->name == "level") {
			osg::ref_ptr<Level> level = new Level;
			level->getOrCreateObjectTypeMap()->parent = otm;
			level->getOrCreateTileTypeMap()->parent = ttm;
			level->getOrCreateAppearanceMap()->parent = am;
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
		ADD_OBJECT_SERIALIZER(appearanceMap, gfx::AppearanceMap, NULL);
		ADD_VECTOR_SERIALIZER(levels, std::vector<osg::ref_ptr<Level> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}
}
