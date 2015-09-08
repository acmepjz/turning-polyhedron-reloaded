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
		std::string map;
		osg::Vec3i pos;
		MapData *_map;
	};

#define UTIL_ADD_MapPosition_GETTER_SETTER(VARNAME) \
	UTIL_ADD_BYREF_GETTER_SETTER2(std::string,VARNAME.map,VARNAME##_map) \
	UTIL_ADD_BYREF_GETTER_SETTER2(osg::Vec3i,VARNAME.pos,VARNAME##_pos)

#define ADD_MapPosition_SERIALIZER(PROP) \
	ADD_STRING_SERIALIZER(PROP##_map,""); \
	ADD_VEC3I_SERIALIZER(PROP##_pos,osg::Vec3i());

}
