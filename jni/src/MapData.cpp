#include "MapData.h"
#include "TileType.h"
#include <osg/Group>
#include <osg/MatrixTransform>

namespace game {

	MapData::MapData()
		: scale(1.0f, 1.0f, 1.0f)
		, step(1.0f, 1.0f, 1.0f)
	{
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
		tiles = tmp;
	}

	osg::Node* MapData::createInstance() {
		osg::ref_ptr<osg::Group> group = new osg::Group;

#define SX(X) s##X = lbound.X()
#define EX(X) e##X = lbound.X() + size.X()
		const int SX(x), EX(x), SX(y), EX(y), SX(z), EX(z);
#undef SX
#undef EX

		osg::Matrix mat0;
		mat0.makeScale(scale);
		mat0.postMultRotate(osg::Quat(rot.x(), osg::X_AXIS, rot.y(), osg::Y_AXIS, rot.z(), osg::Z_AXIS));
		mat0.postMultTranslate(pos);

		int idx = 0;
		for (int z = sz; z < ez; z++) {
			for (int y = sy; y < ey; y++) {
				for (int x = sx; x < ex; x++) {
					TileType *tile = tiles[idx].get();
					if (tile && tile->appearance) {
						osg::Matrix mat;
						mat.makeTranslate(step.x()*x, step.y()*y, step.z()*z);
						mat.postMult(mat0);

						osg::MatrixTransform *trans = new osg::MatrixTransform;
						trans->setMatrix(mat);
						trans->addChild(tile->appearance.get());
						group->addChild(trans);
					}
					idx++;
				}
			}
		}

		return group.release();
	}

}
