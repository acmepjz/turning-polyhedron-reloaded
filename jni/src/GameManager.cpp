#include "GameManager.h"
#include "CompressionManager.h"
#include "LevelCollection.h"
#include "XMLReaderWriter.h"
#include "util_err.h"

#include <iostream>
#include <fstream>

using namespace game;

GameManager* GameManager::instance = NULL;

GameManager::GameManager() {
	instance = this;
}

GameManager::~GameManager() {
	instance = NULL;
}

void GameManager::loadDefaults() {
	defaultObjectTypeMap = new ObjectTypeMap;
	osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultObjectTypes.xml", std::ios::in | std::ios::binary));
	if (!x.valid() || !defaultObjectTypeMap->load(x.get()))
		UTIL_WARN "Failed to load default object types" << std::endl;

	defaultAppearanceMap.clear();
	x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultAppearances.xml", std::ios::in | std::ios::binary));
	if (!x.valid() || !gfx::loadAppearanceMap(x.get(), &defaultAppearanceMap))
		UTIL_WARN "Failed to load default appearances" << std::endl;

	defaultTileTypeMap = new TileTypeMap;
	x = XMLReaderWriter::readFile(
		std::ifstream("../data/DefaultTileTypes.xml", std::ios::in | std::ios::binary));
	if (!x.valid() || !defaultTileTypeMap->load(x.get(), &defaultAppearanceMap))
		UTIL_WARN "Failed to load default tile types" << std::endl;
}

game::Level* GameManager::loadLevel(const char* filename, int levelIndex) {
	if (!filename) return NULL;

	UTIL_NOTICE "Loading level '" << filename << "'" << std::endl;

	std::istream *fin = CompressionManager::instance->openFileForRead(filename);
	if (fin) {
		osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(*fin);
		delete fin;

		if (x.valid()) {
			osg::ref_ptr<osg::Object> obj = LevelCollection::loadLevelOrCollection(x.get(),
				defaultObjectTypeMap, defaultTileTypeMap, &defaultAppearanceMap);

			// check if it is level collection
			LevelCollection *lc = dynamic_cast<LevelCollection*>(obj.get());
			if (lc) {
				if (levelIndex < 0 || levelIndex >= (int)lc->levels.size()) levelIndex = 0;
				osg::ref_ptr<game::Level> level = lc->levels[levelIndex];
				obj = NULL;
				return level.release();
			}

			// check if it is level
			Level *lv = dynamic_cast<Level*>(obj.get());
			if (lv) {
				osg::ref_ptr<game::Level> level = lv;
				obj = NULL;
				return level.release();
			}
		}
	}

	UTIL_ERR "Failed to load level '" << filename << "'" << std::endl;
	return NULL;
}

osg::Object* GameManager::loadLevelOrCollection(const char* filename) {
	if (!filename) return NULL;

	UTIL_NOTICE "Loading file '" << filename << "'" << std::endl;

	std::istream *fin = CompressionManager::instance->openFileForRead(filename);
	if (fin) {
		osg::ref_ptr<XMLNode> x = XMLReaderWriter::readFile(*fin);
		delete fin;

		if (x.valid()) {
			osg::ref_ptr<osg::Object> obj = LevelCollection::loadLevelOrCollection(x.get(),
				defaultObjectTypeMap, defaultTileTypeMap, &defaultAppearanceMap);
			if (obj.valid()) return obj.release();
		}
	}

	UTIL_ERR "Failed to load file '" << filename << "'" << std::endl;
	return NULL;
}

Level* GameManager::loadOrCreateLevel(const char* filename, int levelIndex) {
	// try to load a level
	Level *ptr = loadLevel(filename, levelIndex);
	if (ptr) return ptr;

	UTIL_NOTICE "Creating a default level" << std::endl;

	// create a default level
	osg::ref_ptr<game::Level> level = new game::Level;
	level->name = "Unnamed level";
	level->objectTypeMap = new ObjectTypeMap(*defaultObjectTypeMap);
	level->tileTypeMap = new TileTypeMap(*defaultTileTypeMap);

	//some tile types
	osg::ref_ptr<TileType> ground, ground2, wall, ex;
	ground = level->getOrCreateTileTypeMap()->lookup("ground");
	ground2 = level->getOrCreateTileTypeMap()->lookup("block-ground");
	wall = level->getOrCreateTileTypeMap()->lookup("wall");
	ex = level->getOrCreateTileTypeMap()->lookup("goal");

	//create a map
	osg::ref_ptr<game::MapData> dat = new game::MapData;
	dat->id = "m1";
	dat->resize(osg::Vec3i(), osg::Vec3i(10, 6, 3), false);
	{
		const char s[] =
			"11111     "
			"111111    "
			"11111111W "
			" 111111111"
			"    W11811"
			"      111 ";
		for (int i = 0; i < 60; i++) {
			switch (s[i]) {
			case '1':
				dat->tiles[i] = ground;
				break;
			case '8':
				dat->tiles[i] = ex;
				break;
			case 'W':
				dat->tiles[i] = wall;
				break;
			}
		}
		dat->set(4, 0, 2, ground2.get());
	}
	level->addMapData(dat.get());

	//create a polyhedron (test only)
	osg::ref_ptr<game::Polyhedron> poly = new game::Polyhedron;
	poly->id = "p1";
	poly->flags = poly->MAIN | poly->FRAGILE | poly->SUPPORTER | poly->VISIBLE | poly->FLOATING | poly->CONTINUOUS_HITTEST;
	poly->movement = poly->ROLLING_X | poly->MOVING_Y; //test
	poly->controller = poly->PLAYER;
	poly->pos.map = "m1";
	poly->pos.pos.set(1, 1, 0);
#if 1
	poly->pos.flags = poly->pos.ROT_YXZ | poly->pos.UPPER_Y;
	poly->resize(osg::Vec3i(-1, -1, -1), osg::Vec3i(1, 2, 4), true, false); //test
	poly->customShape[3] = 0;
#endif
	level->addPolyhedron(poly.get());

	/*{
	//create a level collection
	osg::ref_ptr<game::LevelCollection> lvs = new game::LevelCollection;
	lvs->name = "Unnamed level pack";
	lvs->levels.push_back(level);

	//test!!!
	osgDB::writeObjectFile(*lvs, "out.osgt");
	}*/

	return level.release();
}

/*game::Level* test2(){
	osg::ref_ptr<game::Level> level;
	{
		osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile("out.osgt");
		osg::ref_ptr<game::LevelCollection> lvs = dynamic_cast<game::LevelCollection*>(obj.get());
		level = lvs->levels[0];
	}
	return level.release();
}*/
