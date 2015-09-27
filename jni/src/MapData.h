#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <osg/Matrix>
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

	/// The move direction.
	enum MoveDirection {
		MOVE_NEG_X = 0,
		MOVE_POS_X = 1,
		MOVE_NEG_Y = 2,
		MOVE_POS_Y = 3,
		MOVE_NEG_Z = 4,
		MOVE_POS_Z = 5,
		MOVE_LEFT = MOVE_NEG_X,
		MOVE_RIGHT = MOVE_POS_X,
		MOVE_UP = MOVE_NEG_Y,
		MOVE_DOWN = MOVE_POS_Y,
	};

	class Level;

	/// represents a block of map data in a map.

	class MapData :
		public osg::Object
	{
	public:
		/// the shape of map
		enum MapShape {
			RECTANGULAR = 0,
			TRIANGULAR = 3,
		};
	protected:
		virtual ~MapData();
	public:
		META_Object(game, MapData);

		MapData();
		MapData(const MapData& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		///get tile at specified position (with bounds check)
		TileType* get(int x, int y, int z);

		///get tile at specified position (with bounds check)
		TileType* get(const osg::Vec3i& p) {
			return get(p.x(), p.y(), p.z());
		}

		///set tile at specified position (with bounds check)
		void set(int x, int y, int z, TileType* t);

		///set tile at specified position (with bounds check)
		void set(const osg::Vec3i& p, TileType* t) {
			set(p.x(), p.y(), p.z(), t);
		}

		///get or set tile at specified position
		osg::ref_ptr<TileType>& operator()(int x, int y, int z);

		///get or set tile at specified position
		osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}

		///get or set tile at specified position
		const osg::ref_ptr<TileType>& operator()(int x, int y, int z) const;

		///get or set tile at specified position
		const osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		void resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool preserved);

		///calculate the \ref _transform matrix.
		void computeTransform();

		///apply transform to the specified matrix.
		void applyTransform(osg::Matrix& ret) const {
			ret.postMult(_transform);
		}

		///apply transformation matrix of specified position (integer coordinate).
		void applyTransform(const osg::Vec3i& p, osg::Matrix& ret) const {
			ret.postMultTranslate(osg::Vec3(step.x()*p.x(), step.y()*p.y(), step.z()*p.z()));
			applyTransform(ret);
		}

		///apply transformation matrix of specified position (float coordinate).
		void applyTransform(const osg::Vec3& p, osg::Matrix& ret) const {
			ret.postMultTranslate(osg::Vec3(step.x()*p.x(), step.y()*p.y(), step.z()*p.z()));
			applyTransform(ret);
		}

		///test only
		void createInstance();

		void init(Level* parent);

		///check if the position is valid
		bool isValidPosition(int x, int y, int z) const;

		///check if the position is valid
		bool isValidPosition(const osg::Vec3i& pos) const {
			return isValidPosition(pos.x(), pos.y(), pos.z());
		}

	public:
		std::string id; //!< id, used to find this block
		int shape; //!< map shape. \sa MapShape
		osg::Vec3i lbound; //!< lower bound of map data
		osg::Vec3i size; //!< size of map data

		osg::Vec3f pos; //!< position.
		osg::Vec3f rot; //!< rotation (yaw, pitch, roll)
		osg::Vec3f scale; //!< scale
		osg::Vec3f step; //!< step, which determines spaces between individual blocks.

		std::vector<osg::ref_ptr<TileType> > tiles; //!< a 3D array of tiles.

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, shape);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, lbound);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, size);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, rot);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, scale);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, step);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<TileType> >, tiles);

	public:
		//the following properties don't save to file and is generated at runtime
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
		osg::Matrix _transform; //!< the transform
	};

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
		void applyTransform(osg::Matrix& ret) const {
			if (_map) {
				_map->applyTransform(pos, ret);
			} else {
				ret.postMultTranslate(osg::Vec3(pos.x(), pos.y(), pos.z()));
			}
		}

		///move to the adjacent position (no sanity check)
		void move(MoveDirection dir, int count = 1);

		///check if the position is valid
		bool valid() const;
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
