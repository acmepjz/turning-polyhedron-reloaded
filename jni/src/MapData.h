#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/Vec3i>
#include <osg/Vec3f>
#include <string>
#include <vector>
#include "util.h"
#include "TileType.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

namespace game {

	/// represents a block of map data in a map.

	class MapData :
		public osg::Object
	{
	protected:
		virtual ~MapData();
	public:
		META_Object(game, MapData);

		MapData();
		MapData(const MapData& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		osg::ref_ptr<TileType>& operator()(int x, int y, int z);
		osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}
		const osg::ref_ptr<TileType>& operator()(int x, int y, int z) const;
		const osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		void resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool preserved);

		///test only
		osg::Node* createInstance();

	public:
		std::string id; //!< id, used to find this block
		osg::Vec3i lbound; //!< lower bound of map data
		osg::Vec3i size; //!< size of map data

		osg::Vec3f pos; //!< position.
		osg::Vec3f rot; //!< rotation (yaw, pitch, roll)
		osg::Vec3f scale; //!< scale
		osg::Vec3f step; //!< step, which determines spaces between individual blocks.

		std::vector<osg::ref_ptr<TileType> > tiles; //!< a 3D array of tiles.

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, lbound);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, size);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, rot);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, scale);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, step);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<TileType> >, tiles);
	};

	/// represents a position in a map.

	struct MapPosition {
		MapPosition() :
			_map(NULL)
		{
		}
		bool operator!=(const MapPosition& other) const {
			return map != other.map || pos != other.pos;
		}
		std::string map;
		osg::Vec3i pos;

		//the following properties don't save to file and is generated at runtime
		MapData *_map;
	};

	osgDB::InputStream& operator>>(osgDB::InputStream& s, MapPosition& obj);
	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const MapPosition& obj);

}
