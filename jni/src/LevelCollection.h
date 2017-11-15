#pragma once

#include <string>
#include <vector>
#include "util_object.h"
#include "Level.h"
#include "ObjectType.h"
#include "TileType.h"

class XMLNode;

namespace game {

	/// Represents a level collection.

	class LevelCollection :
		public osg::Object
	{
	protected:
		virtual ~LevelCollection();
	public:
		META_Object(game, LevelCollection);

		LevelCollection();
		LevelCollection(const LevelCollection& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		TileTypeMap* getOrCreateTileTypeMap(){
			if (!tileTypeMap.valid()) tileTypeMap = new TileTypeMap;
			return tileTypeMap.get();
		}
		ObjectTypeMap* getOrCreateObjectTypeMap(){
			if (!objectTypeMap.valid()) objectTypeMap = new ObjectTypeMap;
			return objectTypeMap.get();
		}
		gfx::AppearanceMap* getOrCreateAppearanceMap(){
			if (!appearanceMap.valid()) appearanceMap = new gfx::AppearanceMap;
			return appearanceMap.get();
		}

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `levelCollection`

		/** load from XML node, assume the node is called `level` or `levelCollection`
		\param node the XML node
		\param otm the default object types
		\param ttm the default tile types
		\param am the default appearances
		*/
		static osg::Object* loadLevelOrCollection(const XMLNode* node, ObjectTypeMap* otm, TileTypeMap* ttm, gfx::AppearanceMap* am);

	public:
		std::string name; //!< level pack name

		std::vector<osg::ref_ptr<Level> > levels; //!< levels

		osg::ref_ptr<TileTypeMap> tileTypeMap; //!< tile type map used in this level
		osg::ref_ptr<ObjectTypeMap> objectTypeMap; //!< object type map used in this level
		osg::ref_ptr<gfx::AppearanceMap> appearanceMap; //!< appearance map used in this level

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<Level> >, levels);
		UTIL_ADD_OBJ_GETTER_SETTER(TileTypeMap, tileTypeMap);
		UTIL_ADD_OBJ_GETTER_SETTER(ObjectTypeMap, objectTypeMap);
		UTIL_ADD_OBJ_GETTER_SETTER(gfx::AppearanceMap, appearanceMap);
	};

}
