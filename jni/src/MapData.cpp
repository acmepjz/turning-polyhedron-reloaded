#include "MapData.h"
#include "Level.h"
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>

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

	//TODO: consider the shape
	void MapData::resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool preserved){
		if (!preserved) {
			lbound = lbound_;
			size = size_;
			tiles.resize(size_.x()*size_.y()*size_.z());
			return;
		}

		std::vector<osg::ref_ptr<TileType> > tmp = tiles;
		tiles.resize(size_.x()*size_.y()*size_.z());

#define SX(X) s##X = lbound.X() > lbound_.X() ? lbound.X() : lbound_.X()
#define EX(X) e##X = (lbound.X() + size.X() < lbound_.X() + size_.X()) ? \
	(lbound.X() + size.X()) : (lbound_.X() + size_.X())
		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);
#undef SX
#undef EX

		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					int old_idx = ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x();
					int new_idx = ((z - lbound_.z())*size_.y() + y - lbound_.y())*size_.x() + x - lbound_.x();
					tiles[new_idx] = tmp[old_idx];
				}
			}
		}

		lbound = lbound_;
		size = size_;
	}

	void MapData::createInstance() {
		osg::ref_ptr<osg::Group> group = new osg::Group;

#define SX(X) s##X = lbound.X()
#define EX(X) e##X = lbound.X() + size.X()
		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);
#undef SX
#undef EX

		int idx = 0;
		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					TileType *tile = tiles[idx].get();
					if (tile && tile->appearance) {
						osg::Matrix mat;
						mat.makeTranslate(step.x()*x, step.y()*y, step.z()*z);
						applyTransform(mat);

						osg::MatrixTransform *trans = new osg::MatrixTransform;
						trans->setMatrix(mat);
						trans->addChild(tile->appearance.get());
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

	bool MapData::isValidPosition(const osg::Vec3i& pos) const {
		return pos.x() >= lbound.x() && pos.x() < lbound.x() + size.x()
			&& pos.y() >= lbound.y() && pos.y() < lbound.y() + size.y()
			&& pos.z() >= lbound.z() && pos.z() < lbound.z() + size.z();
	}

	void MapData::init(Level* parent){
		computeTransform();

		//check map data size
		{
			size_t n = size.x()*size.y()*size.z();
			size_t m = tiles.size();
			if (m < n) {
				OSG_NOTICE << "[" __FUNCTION__ "] data size mismatch, expected: " << n << ", actual: " << m << std::endl;
				tiles.reserve(n);
				for (; m < n; m++) tiles.push_back(NULL);
			}
		}
	}

	void MapPosition::init(Level* parent){
		Level::MapDataMap::iterator it = parent->maps.find(map);
		if (it != parent->maps.end()) {
			_map = it->second.get();
		} else {
			OSG_NOTICE << "[" __FUNCTION__ "] map id '" << map << " not found" << std::endl;
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
		ADD_VECTOR_SERIALIZER(tiles, std::vector<osg::ref_ptr<TileType> >, osgDB::BaseSerializer::RW_OBJECT, 8);
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
