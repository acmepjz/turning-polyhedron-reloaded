#include "BoxFileLoader.h"
#include "LZSS.h"
#include "u8file.h"
#include "util_err.h"

#include "GameManager.h"
#include "LevelCollection.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#define DECLARE_AND_LOOKUP_APPEARANCE(VAR) gfx::Appearance *VAR = \
	GameManager::instance->defaultAppearanceMap->lookup(#VAR);

struct typeBridge {
	enum enumBridgeBehavior {
		OFF = 0,
		ON = 1,
		TOGGLE = 2,
	};
	int x, y;
	int Behavior;
};

typedef std::vector<typeBridge> typeSwitch;

static bool mycmp(const char* s1, const char* s2, int maxlen) {
	for (int i = 0; i < maxlen; i++) {
		if (s1[i] != s2[i]) return false;
		if (s1[i] == 0) return true;
	}
	return true;
}

static void mycpy(char* dst, const char* src, int maxlen) {
	for (int i = 0; i < maxlen; i++) {
		if ((dst[i] = src[i]) == 0) {
			while ((++i) < maxlen) dst[i] = 0;
			return;
		}
	}
}

bool BoxFile::loadFile(std::istream* f, const char* _signature, bool bSkipSignature) {
	std::stringstream data; // used only when it is compressed
	bool ret = false;

	char _sig[8];
	if (u8fread(_sig, 8, f) == 8) {
		if (bSkipSignature || _signature == NULL || _signature[0] == 0 || mycmp(_sig, _signature, 8)) {
			int len = u8freadLE32(f);
			if (len < 0) {
				// it is LZSS compressed
				int m = u8fseekg(f, 0, SEEK_END) - 12;
				u8fseekg(f, 12, SEEK_SET);

				LZSS lzss;
				lzss.infile = f;
				lzss.outfile = &data;
				lzss.Decode();

				f = &data;
				u8fseekg(f, 0, SEEK_SET);
			}

			// start to read actual data
			nodes.clear();
			ret = true;
			int nodeCount = u8freadLE32(f);
			if (nodeCount > 0) {
				nodes.resize(nodeCount);
				for (int i = 0; i < nodeCount; i++) {
					if (u8fread(nodes[i].name, 4, f) != 4) {
						UTIL_ERR "Unexpected EOF when reading node name" << std::endl;
						ret = false;
						break;
					}
					int m = u8freadLE32(f);
					if (m > 0) {
						nodes[i].nodes.resize(m);
						for (int j = 0; j < m; j++) {
							int m2 = u8freadLE32(f);
							if (m2 > 0) {
								nodes[i].nodes[j].resize(m2);
								if (u8fread(&(nodes[i].nodes[j][0]), m2, f) != m2) {
									UTIL_ERR "Unexpected EOF when reading node data" << std::endl;
									ret = false;
									break;
								}
							}
						}
					}
				}
			}
		} else {
			UTIL_ERR "Signature mismatch" << std::endl;
		}
	} else {
		UTIL_ERR "Unexpected EOF when reading signature" << std::endl;
	}

	return ret;
}

bool BoxFile::saveFile(std::ostream* f, bool IsCompress) {
	// generate actual data
	std::stringstream data;
	u8fwriteLE32(&data, nodes.size());
	for (int i = 0, m = nodes.size(); i < m; i++) {
		u8fwrite(nodes[i].name, 4, &data);
		u8fwriteLE32(&data, nodes[i].nodes.size());
		for (int j = 0, m2 = nodes[i].nodes.size(); j < m2; j++) {
			u8fwriteLE32(&data, nodes[i].nodes[j].size());
			u8fwrite(&(nodes[i].nodes[j][0]), nodes[i].nodes[j].size(), &data);
		}
	}

	int len = u8ftellp(&data);

	// save file
	u8fwrite(signature, 8, f);
	if (IsCompress) {
		u8fwriteLE32(f, -len);

		u8fseekg(&data, 0, SEEK_SET);

		LZSS lzss;
		lzss.infile = &data;
		lzss.outfile = f;
		lzss.Encode();
	} else {
		u8fwriteLE32(f, len);
		u8fwrite(&(data.str()[0]), len, f);
	}

	// over
	return true;
}

BoxFileNodeArray* BoxFile::addNodeArray(const char* name) {
	if (name == NULL) name = "";
	BoxFileNodeArray node;
	mycpy(node.name, name, 4);
	nodes.push_back(node);
	return &(nodes[nodes.size() - 1]);
}

BoxFileNodeArray* BoxFile::findNodeArray(const char* name) {
	for (int i = 0, m = nodes.size(); i < m; i++) {
		if (mycmp(nodes[i].name, name, 4)) {
			return &(nodes[i]);
		}
	}
	return NULL;
}

const BoxFileNodeArray* BoxFile::findNodeArray(const char* name) const {
	for (int i = 0, m = nodes.size(); i < m; i++) {
		if (mycmp(nodes[i].name, name, 4)) {
			return &(nodes[i]);
		}
	}
	return NULL;
}

void BoxFile::setSignature(const char* _signature) {
	mycpy(signature, _signature, 8);
}

game::LevelCollection* BoxFileLoader::loadLevelCollection(const BoxFile& d) {
	const BoxFileNodeArray *array = d.findNodeArray(BOX_LEV);
	if (array == NULL) return NULL;

	enum enumBloxorzTileType {
		EMPTY = 0,
		GROUND = 1,
		SOFT_BUTTON = 2,
		HARD_BUTTON = 3,
		TELEPORTER = 4,
		THIN_GROUND = 5,
		BRIDGE_OFF = 6,
		BRIDGE_ON = 7,
		GOAL = 8,
		ICE = 9,
		PYRAMID = 10,
		WALL = 11,
		TILETYPE_MAX = 12,
	};

	// get predefined types
	game::TileType *boxTileTypes[TILETYPE_MAX] = {};
	for (int i = 1; i < TILETYPE_MAX; i++) {
		boxTileTypes[i] = GameManager::instance->defaultTileTypeMap->lookupByIndex(i);
	}

	// get predefined appearances
	DECLARE_AND_LOOKUP_APPEARANCE(a_cuboid_1x1x1);
	DECLARE_AND_LOOKUP_APPEARANCE(a_cuboid_1x1x2);

	osg::ref_ptr<game::LevelCollection> lc = new game::LevelCollection();

	lc->getOrCreateObjectTypeMap()->parent = GameManager::instance->defaultObjectTypeMap;
	lc->getOrCreateTileTypeMap()->parent = GameManager::instance->defaultTileTypeMap;
	lc->getOrCreateAppearanceMap()->parent = GameManager::instance->defaultAppearanceMap;

	for (int lvnumber = 0, lvm = array->nodes.size(); lvnumber < lvm; lvnumber++) {
		const BoxFileNode &data = array->nodes[lvnumber];
		if (data.empty()) {
			UTIL_WARN "Level '" << (lvnumber + 1) << "' does not contain data" << std::endl;
			lc->levels.push_back(GameManager::instance->createLevel());
			continue;
		}

		std::istringstream f(data);

		std::vector<unsigned char> dat;
		std::vector<int> dat2;

		const int datw = u8freadLE32(&f);
		const int dath = u8freadLE32(&f);
		const int m = datw*dath;
		const int StartX = u8freadLE32(&f);
		const int StartY = u8freadLE32(&f);

		if (datw > 0 && dath > 0) {
			dat.resize(m);
			dat2.resize(m);
			u8fread(&(dat[0]), m, &f);
			for (int i = 0; i < m; i++) dat2[i] = u8freadLE32(&f);
		} else {
			UTIL_WARN "Level '" << (lvnumber + 1) << "' has invalid size" << std::endl;
			lc->levels.push_back(GameManager::instance->createLevel());
			continue;
		}

		std::vector<typeSwitch> switches;
		const int switchCount = u8freadLE32(&f);
		if (switchCount > 0) {
			switches.resize(switchCount);
			for (int i = 0; i < switchCount; i++) {
				int m = u8freadLE32(&f);
				if (m>0) switches[i].resize(m);
				for (int j = 0; j < m; j++) {
					switches[i][j].x = u8freadLE32(&f);
					switches[i][j].y = u8freadLE32(&f);
					switches[i][j].Behavior = u8freadLE32(&f);
				}
			}
		}

		// generate level from loaded data
		osg::ref_ptr<game::Level> lv = new game::Level;
		lv->getOrCreateObjectTypeMap()->parent = lc->objectTypeMap;
		lv->getOrCreateTileTypeMap()->parent = lc->tileTypeMap;
		lv->getOrCreateAppearanceMap()->parent = lc->appearanceMap;
		lc->levels.push_back(lv);

		osg::ref_ptr<game::MapData> md = new game::MapData;
		md->id = "m";
		md->resize(osg::Vec3i(), osg::Vec3i(datw, dath, 1), false);
		for (int i = 0; i < m; i++) {
			unsigned char c = dat[i];
			if (c > 0 && c < TILETYPE_MAX) md->tiles[i] = boxTileTypes[c];
		}
		
		lv->addMapData(md);

		// create polyhedra
		osg::ref_ptr<game::Polyhedron> poly = new game::Polyhedron;
		poly->id = "p";
		poly->flags = poly->MAIN | poly->FRAGILE | poly->TILTABLE | poly->VISIBLE | poly->CONTINUOUS_HITTEST;
		poly->movement = poly->ROLLING_ALL;
		poly->controller = poly->PLAYER;
		poly->pos.map = "m";
		poly->pos.pos.set(StartX - 1, StartY - 1, 0);
		poly->getOrCreateAppearanceMap()->map[""] = a_cuboid_1x1x2;
		poly->pos.flags = poly->pos.ROT_XYZ;
		poly->resize(osg::Vec3i(), osg::Vec3i(1, 1, 2), false, false);
		lv->addPolyhedron(poly);

		poly = new game::Polyhedron;
		poly->id = "p1";
		poly->flags = poly->FRAGILE;
		poly->movement = poly->ROLLING_ALL;
		poly->controller = poly->PLAYER;
		poly->pos.map = "m";
		poly->getOrCreateAppearanceMap()->map[""] = a_cuboid_1x1x1;
		poly->pos.flags = poly->pos.ROT_XYZ;
		poly->resize(osg::Vec3i(), osg::Vec3i(1, 1, 1), false, false);
		lv->addPolyhedron(poly);

		poly = new game::Polyhedron;
		poly->id = "p2";
		poly->flags = poly->FRAGILE;
		poly->movement = poly->ROLLING_ALL;
		poly->controller = poly->PLAYER;
		poly->pos.map = "m";
		poly->getOrCreateAppearanceMap()->map[""] = a_cuboid_1x1x1;
		poly->pos.flags = poly->pos.ROT_XYZ;
		poly->resize(osg::Vec3i(), osg::Vec3i(1, 1, 1), false, false);
		lv->addPolyhedron(poly);

		// create polyhedron merge
		osg::ref_ptr<game::PolyhedronMerge> pm = new game::PolyhedronMerge;
		pm->src.insert("p1");
		pm->src.insert("p2");
		pm->dest = "p";
		lv->polyhedronMerge.push_back(pm);

		// create events
		std::vector<osg::ref_ptr<game::TileProperty> > tileProps;

		if (switchCount > 0) {
			for (int i = 0; i < switchCount; i++) {
				osg::ref_ptr<game::TileProperty> tp = new game::TileProperty;
				osg::ref_ptr<game::EventHandler> eh = new game::EventHandler;

				eh->type = eh->ON_EVENT;
				eh->conditions["type"] = "pressed";

				for (int j = 0; j < (int)switches[i].size(); j++) {
					osg::ref_ptr<game::EventAction> ea = new game::EventAction;
					ea->type = ea->RAISE_EVENT;
					switch (switches[i][j].Behavior) {
					case typeBridge::ON:
						ea->arguments["type"] = "on";
						break;
					case typeBridge::OFF:
						ea->arguments["type"] = "off";
						break;
					default:
						ea->arguments["type"] = "toggle";
						break;
					}
					
					std::ostringstream ss;
					ss << "m(" << (switches[i][j].x - 1) << "," << (switches[i][j].y - 1) << ")";
					ea->arguments["target"] = ss.str();

					eh->actions.push_back(ea);
				}

				tp->events.push_back(eh);

				tileProps.push_back(tp);
			}
		}

		for (int i = 0; i < m; i++) {
			switch (dat[i]) {
			case SOFT_BUTTON:
			case HARD_BUTTON:
				if (dat2[i] > 0 && dat2[i] <= switchCount) {
					md->tileProperties[i] = tileProps[dat2[i] - 1];
				}
				break;
			case TELEPORTER:
				if (dat2[i]) {
					int x1 = (dat2[i] & 0xFF) - 1;
					int y1 = ((dat2[i] >> 8) & 0xFF) - 1;
					int x2 = ((dat2[i] >> 16) & 0xFF) - 1;
					int y2 = ((dat2[i] >> 24) & 0xFF) - 1;

					if (x1 >= 0 && x1 < datw && y1 >= 0 && y1 < dath && x2 >= 0 && x2 < datw && y2 >= 0 && y2 < dath) {
						osg::ref_ptr<game::TileProperty> tp = new game::TileProperty;
						osg::ref_ptr<game::EventHandler> eh = new game::EventHandler;
						eh->type = eh->ON_EVENT;
						eh->conditions["type"] = "pressed";

						osg::ref_ptr<game::EventAction> ea = new game::EventAction;
						ea->type = ea->TELEPORT_POLYHEDRON;

						std::ostringstream ss;
						if (x1 == x2 && y1 == y2) {
							ss << "m(" << x1 << "," << y1 << ")";
							ea->arguments["src"] = "p";
							ea->arguments["dest"] = ss.str();
							ea->arguments["size"] = "1,1";
						} else if (x1 == x2 && (y1 + 1 == y2 || y1 - 1 == y2)) {
							ss << "m(" << x1 << "," << std::min(y1, y2) << ")";
							ea->arguments["src"] = "p";
							ea->arguments["dest"] = ss.str();
							ea->arguments["size"] = "1,2";
						} else if (y1 == y2 && (x1 + 1 == x2 || x1 - 1 == x2)) {
							ss << "m(" << std::min(x1, x2) << "," << y1 << ")";
							ea->arguments["src"] = "p";
							ea->arguments["dest"] = ss.str();
							ea->arguments["size"] = "2,1";
						} else {
							ss << "m(" << x1 << "," << y1 << ")";
							ea->arguments["hide"] = "p";
							ea->arguments["src"] = "p1";
							ea->arguments["dest"] = ss.str();
							ea->arguments["size"] = "1,1";
							eh->actions.push_back(ea);

							ss.str(std::string());
							ea = new game::EventAction(*ea.get());
							ss << "m(" << x2 << "," << y2 << ")";
							ea->arguments["src"] = "p2";
							ea->arguments["dest"] = ss.str();
						}
						eh->actions.push_back(ea);

						tp->events.push_back(eh);

						md->tileProperties[i] = tp;
					}
				}
			}
		}
	}

	return lc.release();
}