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

	void loadDefaults();
	game::Level* loadLevel(const char* filename, int levelIndex);

	//! this is test only
	game::Level* loadOrCreateLevel(const char* filename, int levelIndex);

public:
	osg::ref_ptr<game::ObjectTypeMap> defaultObjectTypeMap;
	osg::ref_ptr<game::TileTypeMap> defaultTileTypeMap;
};

extern GameManager* gameMgr;
