#pragma once

#include <osg/Referenced>
#include "ObjectType.h"
#include "TileType.h"

namespace game {
	class Level;
}

/** This class saves some global data used in game (level loading, etc.)
*/
class GameManager : public osg::Referenced {
protected:
	virtual ~GameManager() {}
public:
	GameManager() {}

	/** load default object and tile type map, etc. */
	void loadDefaults();

	/** load a level or a level in a level collection */
	game::Level* loadLevel(const char* filename, int levelIndex);

	/** load a level or a level collection */
	osg::Object* loadLevelOrCollection(const char* filename);

	/** this is test only */
	game::Level* loadOrCreateLevel(const char* filename, int levelIndex);

public:
	osg::ref_ptr<game::ObjectTypeMap> defaultObjectTypeMap;
	osg::ref_ptr<game::TileTypeMap> defaultTileTypeMap;
};

extern GameManager* gameMgr;
