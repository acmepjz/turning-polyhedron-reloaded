#pragma once

#include <osg/Node>
#include <osg/Matrix>
#include <osg/Vec3i>
#include <osg/Vec3f>
#include <string>
#include <vector>
#include "util_object.h"
#include "TileType.h"
#include "TileProperty.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

class XMLNode;

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
	class EventDescription;

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

		/** set tile at specified position (with bounds check) and update the graphics instance
		\note Only call this function when the graphics instance is created
		*/
		void substituteTileType(int x, int y, int z, TileType* t);

		/** get or set tile at specified position
		\warning no array bounds check
		*/
		osg::ref_ptr<TileType>& operator()(int x, int y, int z);

		/** get or set tile at specified position
		\warning no array bounds check
		*/
		osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}

		/** get tile at specified position
		\warning no array bounds check
		*/
		const osg::ref_ptr<TileType>& operator()(int x, int y, int z) const;

		/** get tile at specified position
		\warning no array bounds check
		*/
		const osg::ref_ptr<TileType>& operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		///get tile property at specified position (with bounds check)
		TileProperty* getProp(int x, int y, int z);

		///get tile property at specified position (with bounds check)
		TileProperty* getProp(const osg::Vec3i& p) {
			return getProp(p.x(), p.y(), p.z());
		}

		///set tile property at specified position (with bounds check)
		void setProp(int x, int y, int z, TileProperty* t);

		///set tile property at specified position (with bounds check)
		void setProp(const osg::Vec3i& p, TileProperty* t) {
			setProp(p.x(), p.y(), p.z(), t);
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

		/// creates \ref _appearance (test only)
		void createInstance(bool isEditMode);

		void init(Level* parent);

		bool load(const XMLNode* node, Level* parent); //!< load from XML node, assume the node is called `mapData`

		///check if the position is valid
		bool isValidPosition(int x, int y, int z) const {
			return x >= lbound.x() && x < lbound.x() + size.x()
				&& y >= lbound.y() && y < lbound.y() + size.y()
				&& z >= lbound.z() && z < lbound.z() + size.z();
		}

		///check if the position is valid
		bool isValidPosition(const osg::Vec3i& pos) const {
			return isValidPosition(pos.x(), pos.y(), pos.z());
		}

		/** find the first tile with specified tag.
		\param[in] tag The tag.
		\param[out] ret The result position.
		\return Found or not.
		*/
		bool findTag(const std::string& tag, osg::Vec3i& ret) const;

		/** find all tiles with specified tag.
		\param[in] tag The tag.
		\param[out] ret The result position.
		*/
		void findAllTags(const std::string& tag, std::vector<osg::Vec3i>& ret) const;

		void processEvent(Level* parent, EventDescription* evt);

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
		std::vector<osg::ref_ptr<TileProperty> > tileProperties; //!< a 3D array of tile properties.

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, shape);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, lbound);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, size);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, rot);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, scale);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3f, step);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<TileType> >, tiles);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<TileProperty> >, tileProperties);

	public:
		//the following properties don't save to file and is generated at runtime
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
		osg::Matrix _transform; //!< the transform

		std::vector<int> _apprIndex;
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
