#pragma once

#include <osg/Referenced>
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

		std::string id; //!< id, used to find this block
		osg::Vec3i lbound; //!< lower bound of map data
		osg::Vec3i size; //!< size of map data

		osg::Vec3f pos; //!< position of \c center in the world coordinate.
		osg::Vec3f rot; //!< rotation (yaw, pitch, roll)
		osg::Vec3f scale; //!< scale
		osg::Vec3f center; //!< center (relative to the bounding box)
		osg::Vec3f step; //!< step, which determines spaces between individual blocks.

		std::vector<osg::ref_ptr<TileType> > tiles; //!< a 3D array of tiles.
	};

}
