#pragma once

#include <osg/Referenced>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/Vec3i>
#include <osg/Vec3f>
#include <string>
#include <vector>

namespace game {

	class TileType;

	/// represents a block of map data in a map.

	class MapData :
		public osg::Referenced
	{
	protected:
		virtual ~MapData();
	public:
		MapData();

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
	};

	/// represents a position in a map.

	struct MapPosition {
		MapData *map;
		osg::Vec3i pos;
	};

}
