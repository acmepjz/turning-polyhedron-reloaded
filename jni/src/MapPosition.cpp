#include "MapPosition.h"
#include "Level.h"
#include "util_err.h"
#include "util_misc.h"
#include "XMLReaderWriter.h"
#include <osgDB/ObjectWrapper>

namespace game {

	void MapPosition::applyTransform(osg::Matrix& ret) const {
		if (_map) {
			_map->applyTransform(pos, ret);
		} else {
			ret.postMultTranslate(osg::Vec3(pos.x(), pos.y(), pos.z()));
		}
	}

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
