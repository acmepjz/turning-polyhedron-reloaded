#pragma once

#include <osg/Matrix>
#include <osg/Vec3i>
#include <string>

namespace osgDB {
	class InputStream;
	class OutputStream;
}

namespace game {

	/// The move direction.
	enum MoveDirection {
		MOVE_NEG_X = 0, //!< x--
		MOVE_POS_X = 1, //!< x++
		MOVE_NEG_Y = 2, //!< y--
		MOVE_POS_Y = 3, //!< y++
		MOVE_NEG_Z = 4, //!< z--
		MOVE_POS_Z = 5, //!< z++
		MOVE_LEFT = MOVE_NEG_X, //!< x--
		MOVE_RIGHT = MOVE_POS_X, //!< x++
		MOVE_UP = MOVE_NEG_Y, //!< y--
		MOVE_DOWN = MOVE_POS_Y, //!< y++
	};

	class Level;
	class MapData;

	/// represents a position in a map.

	class MapPosition {
	public:
		MapPosition() :
			_map(NULL)
		{
		}
		///ad-hoc, don't use
		bool operator!=(const MapPosition& other) const {
			return map != other.map || pos != other.pos;
		}
		void init(Level* parent);

		///apply transform to the specified matrix.
		void applyTransform(osg::Matrix& ret) const;

		///move to the adjacent position (no sanity check)
		void move(MoveDirection dir, int count = 1);

		///check if the position is valid
		bool valid() const;

		bool load(const std::string& data, Level* parent, MapData* mapData); //!< load from a string in XML node,

	public:
		std::string map;
		osg::Vec3i pos;

	public:
		//the following properties don't save to file and is generated at runtime
		MapData *_map;
	};

	osgDB::InputStream& operator>>(osgDB::InputStream& s, MapPosition& obj);
	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const MapPosition& obj);

}
