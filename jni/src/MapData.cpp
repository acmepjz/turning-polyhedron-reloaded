#include "MapData.h"
#include "Level.h"
#include "util_err.h"
#include "util_misc.h"
#include "XMLReaderWriter.h"
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>

#define SX(X) s##X = lbound.X()
#define EX(X) e##X = lbound.X() + size.X()
#define SXRESIZE(X) s##X = lbound.X() > lbound_.X() ? lbound.X() : lbound_.X()
#define EXRESIZE(X) e##X = (lbound.X() + size.X() < lbound_.X() + size_.X()) ? \
	(lbound.X() + size.X()) : (lbound_.X() + size_.X())

namespace game {

	MapData::MapData()
		: shape(0)
		, size(1, 1, 1)
		, scale(1.0f, 1.0f, 1.0f)
		, step(1.0f, 1.0f, 1.0f)
		, tiles(1)
	{
	}

	MapData::MapData(const MapData& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, id(other.id)
		, shape(other.shape)
		, lbound(other.lbound)
		, size(other.size)
		, pos(other.pos)
		, rot(other.rot)
		, scale(other.scale)
		, step(other.step)
	{
		util::copyVector(tiles, other.tiles, copyop);
		util::copyVector(tileProperties, other.tileProperties, copyop, true);
	}

	MapData::~MapData()
	{
	}

	osg::ref_ptr<TileType>& MapData::operator()(int x, int y, int z) {
		int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
		return tiles[idx];
	}

	const osg::ref_ptr<TileType>& MapData::operator()(int x, int y, int z) const {
		int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
		return tiles[idx];
	}

	TileType* MapData::get(int x, int y, int z) {
		if (isValidPosition(x, y, z)) {
			int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
			return tiles[idx].get();
		} else {
			return NULL;
		}
	}

	void MapData::set(int x, int y, int z, TileType* t) {
		if (isValidPosition(x, y, z)) {
			int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
			tiles[idx] = t;
		} else {
			//prevent memory leak
			osg::ref_ptr<TileType> tmp = t;
		}
	}

	// TODO: animation
	void MapData::substituteTileType(int x, int y, int z, TileType* t) {
		int idx = 0;
		TileType *old = NULL;
		if (isValidPosition(x, y, z) &&
			((old = tiles[idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x()].get()) != t)) {

			osg::Group *group = dynamic_cast<osg::Group*>(_appearance.get());
			if (group) {
				osg::MatrixTransform *trans = NULL;
				int apprIndex = _apprIndex[idx];
				if (apprIndex >= 0) trans = dynamic_cast<osg::MatrixTransform*>(group->getChild(apprIndex));

				// if new is NULL then hide the old one
				if (t == NULL) {
					if (trans) trans->setNodeMask(0);
				} else {
					if (trans) {
						trans->setNodeMask(-1);
					} else {
						// if old is NULL or something goes wrong we create a new MatrixTransform
						osg::Matrix mat;
						mat.makeTranslate(step.x()*x, step.y()*y, step.z()*z);
						applyTransform(mat);

						trans = new osg::MatrixTransform;
						trans->setMatrix(mat);

						_apprIndex[idx] = group->getNumChildren();
						group->addChild(trans);
					}
					trans->removeChildren(0, trans->getNumChildren());
					trans->addChild(t->getOrCreateInstance(shape, false)); // FIXME: assume it is not edit mode
				}
			}

			// finally update it
			tiles[idx] = t;
		} else {
			//prevent memory leak
			osg::ref_ptr<TileType> tmp = t;
		}
	}

	TileProperty* MapData::getProp(int x, int y, int z) {
		if (isValidPosition(x, y, z)) {
			int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
			return tileProperties[idx].get();
		} else {
			return NULL;
		}
	}

	void MapData::setProp(int x, int y, int z, TileProperty* t) {
		if (isValidPosition(x, y, z)) {
			int idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
			tileProperties[idx] = t;
		} else {
			//prevent memory leak
			osg::ref_ptr<TileProperty> tmp = t;
		}
	}

	//TODO: consider the shape
	void MapData::resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool preserved){
		if (!preserved) {
			lbound = lbound_;
			size = size_;
			tiles.resize(size_.x()*size_.y()*size_.z());
			tileProperties.resize(size_.x()*size_.y()*size_.z());
			return;
		}

		std::vector<osg::ref_ptr<TileType> > tmp;
		std::vector<osg::ref_ptr<TileProperty> > tmp2;
		std::swap(tmp, tiles);
		std::swap(tmp2, tileProperties);
		tiles.resize(size_.x()*size_.y()*size_.z());
		tileProperties.resize(size_.x()*size_.y()*size_.z());

		const int SXRESIZE(x), EXRESIZE(x), SXRESIZE(y), EXRESIZE(y), SXRESIZE(z), EXRESIZE(z);

		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					int old_idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
					int new_idx = ((z - lbound_.z())*size_.y() + y - lbound_.y())*size_.x() + x - lbound_.x();
					tiles[new_idx] = tmp[old_idx];
					tileProperties[new_idx] = tmp2[old_idx];
				}
			}
		}

		lbound = lbound_;
		size = size_;
	}

	void MapData::createInstance(bool isEditMode) {
		osg::ref_ptr<osg::Group> group = new osg::Group;

		_apprIndex.clear();
		_apprIndex.resize(size.x()*size.y()*size.z(), -1);

		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);

		int idx = 0;
		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					TileType *tile = tiles[idx].get();
					osg::Node *node = tile ? tile->getOrCreateInstance(shape, isEditMode) : NULL;
					if (node) {
						osg::Matrix mat;
						mat.makeTranslate(step.x()*x, step.y()*y, step.z()*z);
						applyTransform(mat);

						osg::MatrixTransform *trans = new osg::MatrixTransform;
						trans->setMatrix(mat);
						trans->addChild(node);

						_apprIndex[idx] = group->getNumChildren();
						group->addChild(trans);
					}
					idx++;
				}
			}
		}

		_appearance = group;
	}

	void MapData::computeTransform() {
		_transform.makeScale(scale);
		_transform.postMultRotate(osg::Quat(rot.x(), osg::X_AXIS, rot.y(), osg::Y_AXIS, rot.z(), osg::Z_AXIS));
		_transform.postMultTranslate(pos);
	}

	void MapData::init(Level* parent){
		_checkpointCount = 0;

		computeTransform();

		//check map data size
		const size_t n = size.x()*size.y()*size.z();
		size_t m = tiles.size();
		if (m < n) {
			UTIL_WARN "tile data size mismatch, expected: " << n << ", actual: " << m << std::endl;
			tiles.reserve(n);
			for (; m < n; m++) tiles.push_back(NULL);
		}

		m = tileProperties.size();
		if (m < n) {
			UTIL_WARN "tile property data size mismatch, expected: " << n << ", actual: " << m << std::endl;
			tileProperties.reserve(n);
			for (; m < n; m++) tileProperties.push_back(NULL);
		}

		// calculate checkpoint count
		for (size_t i = 0; i < n; i++) {
			if (tiles[i].valid() && (tiles[i]->flags & TileType::TileFlags::CHECKPOINT) != 0) {
				_checkpointCount++;
			}
		}
	}

	bool MapData::findTag(const std::string& tag, osg::Vec3i& ret) const {
		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);

		int idx = 0;
		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					TileProperty *prop = tileProperties[idx].get();
					if (prop && prop->tags.find(tag) != prop->tags.end()) {
						ret.set(x, y, z);
						return true;
					}
					idx++;
				}
			}
		}

		return false;
	}

	void MapData::findAllTags(const std::string& tag, std::vector<osg::Vec3i>& ret) const {
		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);

		int idx = 0;
		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					TileProperty *prop = tileProperties[idx].get();
					if (prop && prop->tags.find(tag) != prop->tags.end()) {
						ret.push_back(osg::Vec3i(x, y, z));
					}
					idx++;
				}
			}
		}
	}

	void MapData::processEvent(Level* parent, EventDescription* evt) {
		TileType *tt = get(evt->position);
		if (tt) tt->processEvent(parent, evt);
		TileProperty *prop = getProp(evt->position);
		if (prop) prop->processEvent(parent, evt);
	}

	struct MapDataItem {
		osg::Vec3i pos;
		int count;
		TileType* tile;
		osg::ref_ptr<TileProperty> prop;
	};

	bool MapData::load(const XMLNode* node, Level* parent) {
		id = node->getAttr("id", std::string());
		if (id.empty()) {
			UTIL_WARN "object doesn't have id" << std::endl;
			return false;
		}

		//get shape
		{
			std::string s = node->getAttr("shape", std::string("rect"));
			if (s == "rect") shape = RECTANGULAR;
			else {
				UTIL_WARN "unrecognized shape: " << s << std::endl;
				return false;
			}
		}

		pos = node->getAttrOsgVec("p", osg::Vec3());
		rot = node->getAttrOsgVec("r", osg::Vec3());
		scale = node->getAttrOsgVec("s", osg::Vec3(1, 1, 1));
		step = node->getAttrOsgVec("step", osg::Vec3(1, 1, 1));

		lbound = node->getAttrOsgVec("lbound", osg::Vec3i());
		bool hasSize = node->attributes.find("size") != node->attributes.end();
		int theSize = 0;
		if (hasSize) {
			size = node->getAttrOsgVec("size", osg::Vec3i(1, 1, 1));
			if (size.x() <= 0 || size.y() <= 0 || size.z() <= 0) {
				UTIL_WARN "invalid size" << std::endl;
				return false;
			}
			theSize = size.x()*size.y()*size.z();
			tiles.resize(theSize);
			tileProperties.resize(theSize);
		} else {
			size.set(1, 1, 1);
		}

		//get property from property index
		std::map<int, osg::ref_ptr<TileProperty> > propIndexMap;

		//used when size is unspecified
		std::vector<MapDataItem> mapDataItems;

		//load subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "typeArray") {
				//this node contains map data array
				/*
				format: [<index>|<id>[@<new_index>]]["["<property_index>|<tag>"]"]["*"<count>]
				","=next pos (x++)
				";"=next row (y++)
				"|"=next plane (z++)
				<new_index> should be a negative integer
				*/
				const std::string& contents = subnode->contents;
				std::string::size_type lps = 0;
#define GET_CHARACTER() util::getCharacter(contents, lps)

				int c = 0;
				MapDataItem current;

				for (;;) {
					//get id
					std::string idOrIndex;
					for (;;) {
						c = GET_CHARACTER();
						if (c == '@' || c == '[' || c == '*' || c == ',' || c == ';' || c == '|' || c == EOF) break;
						idOrIndex.push_back(c);
					}

					//get new index
					int newIndex = 0;
					if (c == '@') {
						std::string s;
						for (;;) {
							c = GET_CHARACTER();
							if (c == '[' || c == '*' || c == ',' || c == ';' || c == '|' || c == EOF) break;
							s.push_back(c);
						}
						if (!s.empty()) newIndex = atoi(s.c_str());
					}

					//get tag
					std::string propOrTag;
					if (c == '[') {
						for (;;) {
							c = GET_CHARACTER();
							if (c == ']' || c == EOF) break;
							propOrTag.push_back(c);
						}
						c = GET_CHARACTER();
					}

					//get count
					current.count = 1;
					if (c == '*') {
						std::string s;
						for (;;) {
							c = GET_CHARACTER();
							if (c == ',' || c == ';' || c == '|' || c == EOF) break;
							s.push_back(c);
						}
						if (!s.empty()) current.count = atoi(s.c_str());
					}

					//get tile type from id
					current.tile = parent->getOrCreateTileTypeMap()->lookup(idOrIndex);

					//add new tile mapping
					if (newIndex) parent->getOrCreateTileTypeMap()->addTileMapping(current.tile, newIndex);

					//add property
					current.prop = NULL;
					if (!propOrTag.empty()) {
						if (util::isNumeric(propOrTag)) {
							//it is prop index
							int propIndex = atoi(propOrTag.c_str());
							current.prop = propIndexMap[propIndex];
							if (!current.prop.valid()) {
								current.prop = new TileProperty;
								propIndexMap[propIndex] = current.prop;
							}
						} else {
							//it is tag
							current.prop = new TileProperty;
							current.prop->setTags(propOrTag);
						}
					}

					//put these data to array, and advance to next position
					if (hasSize) {
						int idx = (current.pos.z()*size.y() + current.pos.y())*size.x() + current.pos.x();
						for (int i = 0; i < current.count; i++) {
							if (idx < theSize) {
								tiles[idx] = current.tile;
								tileProperties[idx] = current.prop;
							}
							idx++;

							util::typeArrayAdvance(current.pos, size, i == current.count - 1, c);
						}
					} else {
						//size is unspecified
						mapDataItems.push_back(current);
						current.pos.x() += current.count;

						//extend size
						if (current.pos.x() > size.x()) size.x() = current.pos.x();
						if (current.pos.y() >= size.y()) size.y() = current.pos.y() + 1;
						if (current.pos.z() >= size.z()) size.z() = current.pos.z() + 1;

						if (c == ';') {
							current.pos.x() = 0;
							current.pos.y()++;
						} else if (c == '|') {
							current.pos.x() = 0;
							current.pos.y() = 0;
							current.pos.z()++;
						}
					}

					if (c == EOF) break;
					if (c != ',' && c != ';' && c != '|') {
						UTIL_WARN "unexpected character: '" << char(c) << "', expected ',' or ';' or '|'" << std::endl;
						break;
					}
				}
			} else if (subnode->name == "property") {
				//this node contains a property
				int index = subnode->getAttr("index", 0);

				//lode node only if the index is used, otherwise ignore it
				std::map<int, osg::ref_ptr<TileProperty> >::iterator it = propIndexMap.find(index);
				if (it != propIndexMap.end()) {
					it->second->load(subnode);
				}
			} else if (subnode->name == "polyhedron") {
				osg::ref_ptr<Polyhedron> poly = new Polyhedron;
				if (poly->load(subnode, parent, this)) {
					parent->addPolyhedron(poly.get());
				} else {
					UTIL_WARN "failed to load polyhedron" << std::endl;
				}
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		//put map data when size is unspecified
		if (!hasSize) {
			theSize = size.x()*size.y()*size.z();
			tiles.resize(theSize);
			tileProperties.resize(theSize);

			for (size_t i = 0; i < mapDataItems.size(); i++) {
				MapDataItem &current = mapDataItems[i];

				int idx = (current.pos.z()*size.y() + current.pos.y())*size.x() + current.pos.x();
				for (int i = 0; i < current.count; i++) {
					tiles[idx] = current.tile;
					tileProperties[idx] = current.prop;
					idx++;
				}
			}
		}

		//over
		return true;
	}
#undef GET_CHARACTER

	void MapPosition::init(Level* parent){
		Level::MapDataMap::iterator it = parent->maps.find(map);
		if (it != parent->maps.end()) {
			_map = it->second.get();
		} else {
			UTIL_WARN "map id '" << map << " not found" << std::endl;
			_map = NULL;
		}
	}

	void MapPosition::move(MoveDirection dir, int count){
		//TODO: adjacency
		switch (dir) {
		case MOVE_NEG_X: pos.x() -= count; break;
		case MOVE_POS_X: pos.x() += count; break;
		case MOVE_NEG_Y: pos.y() -= count; break;
		case MOVE_POS_Y: pos.y() += count; break;
		case MOVE_NEG_Z: pos.z() -= count; break;
		case MOVE_POS_Z: pos.z() += count; break;
		}
	}

	bool MapPosition::valid() const{
		return _map && _map->isValidPosition(pos);
	}

	bool MapPosition::load(const std::string& data, Level* parent, MapData* mapData){
		std::string::size_type lps = 0;
#define GET_CHARACTER() util::getCharacter(data, lps)

		int c = 0;

		pos.set(0, 0, 0);

		/* format is
		(<id>|<index>)["("<subscript>...")"|"."<tag>] (if mapData == NULL)
		or [<subscript>...|["."]<tag>] (if mapData != NULL)
		*/

		if (mapData) {
			map = mapData->id;

			std::string s;
			int i = 0;
			bool isTag = false;

			for (;;) {
				c = GET_CHARACTER();
				i++;
				if (c == '.' && i == 1) {
					//skip the first '.'
					isTag = true;
					continue;
				}
				if (c == ':' || c == EOF) break;
				s.push_back(c);
				if (c == '+' || c == '-' || (c >= '0' && c <= '9') || c == ',') {
					//do nothing
				} else {
					isTag = true;
				}
			}

			if (isTag) {
				if (!mapData->findTag(s, pos)) {
					UTIL_WARN "tag '" << s << " not found in map '" << map << "'" << std::endl;
				}
			} else {
				pos = util::getAttrFromStringOsgVec(s, osg::Vec3i());
			}

			return true;
		}

		//get map id
		map.clear();
		for (;;) {
			c = GET_CHARACTER();
			if (c == '(' || c == '.' || c == ':' || c == EOF) break;
			map.push_back(c);
		}

		//check type
		if (c == '(') {
			//it is subscript
			std::string s;
			for (;;) {
				c = GET_CHARACTER();
				if (c == ')' || c == EOF) break;
				s.push_back(c);
			}
			pos = util::getAttrFromStringOsgVec(s, osg::Vec3i());
		} else if (c == '.') {
			//it is tag
			std::string tag;
			for (;;) {
				c = GET_CHARACTER();
				if (c == ':' || c == EOF) break;
				tag.push_back(c);
			}

			Level::MapDataMap::iterator it = parent->maps.find(map);
			if (it != parent->maps.end()) {
				if (!it->second->findTag(tag, pos)) {
					UTIL_WARN "tag '" << tag << " not found in map '" << map << "'" << std::endl;
				}
			} else {
				UTIL_WARN "map id '" << map << " not found" << std::endl;
			}
		} else {
			//unsupported, assume it is (0,0,0)
			if (c >= 0) {
				UTIL_WARN "unexpected character: '" << char(c) << "', expected: '(' or '.'" << std::endl;
			}
		}

		return true;
	}
#undef GET_CHARACTER

	REG_OBJ_WRAPPER(game, MapData, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(shape, 0);
		ADD_VEC3I_SERIALIZER(lbound, osg::Vec3i());
		ADD_VEC3I_SERIALIZER(size, osg::Vec3i(1, 1, 1));
		ADD_VEC3F_SERIALIZER(pos, osg::Vec3f());
		ADD_VEC3F_SERIALIZER(rot, osg::Vec3f());
		ADD_VEC3F_SERIALIZER(scale, osg::Vec3f(1.0f, 1.0f, 1.0f));
		ADD_VEC3F_SERIALIZER(step, osg::Vec3f(1.0f, 1.0f, 1.0f));
		ADD_VECTOR_SERIALIZER(tiles, std::vector<osg::ref_ptr<TileType> >, osgDB::BaseSerializer::RW_OBJECT, -1);
		ADD_VECTOR_SERIALIZER(tileProperties, std::vector<osg::ref_ptr<TileProperty> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}

	osgDB::InputStream& operator>>(osgDB::InputStream& s, MapPosition& obj){
		s.readWrappedString(obj.map);
		s >> obj.pos;
		return s;
	}

	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const MapPosition& obj){
		s.writeWrappedString(obj.map);
		s << obj.pos;
		return s;
	}

}
