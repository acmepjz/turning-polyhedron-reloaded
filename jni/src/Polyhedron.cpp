#include "Polyhedron.h"
#include "ObjectType.h"
#include "Level.h"
#include "SimpleGeometry.h"
#include "Rect.h"
#include "util_err.h"
#include "util_misc.h"
#include "XMLReaderWriter.h"
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgGA/GUIEventHandler>
#include <osgDB/ObjectWrapper>
#include <math.h>
#include <string.h>

namespace game {

	void PolyhedronPosition::init(Level* parent){
		MapPosition::init(parent);
	}

	void PolyhedronPosition::getCurrentPos(int flags, const Polyhedron* poly, Idx& ret) {
		osg::Vec3i currentOrigin(
			(flags & 1) ? (poly->size.x() - 1) : 0,
			(flags & 2) ? (poly->size.y() - 1) : 0,
			(flags & 4) ? (poly->size.z() - 1) : 0);
		ret.origin = (currentOrigin.z()*poly->size.y() + currentOrigin.y())*poly->size.x() + currentOrigin.x(); //new origin

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			ret.size[i] = poly->size[idx]; //new size of dimension i

			int delta = ((flags >> idx) & 1) ? -1 : 1;
			if (idx >= 1) delta *= poly->size.x();
			if (idx >= 2) delta *= poly->size.y();
			ret.delta[i] = delta; //new delta
		}
	}

	void PolyhedronPosition::getCurrentPos(int flags, const Polyhedron* poly, Pos& ret) {
		osg::Vec3i currentOrigin(
			(flags & 1) ? (poly->size.x() - 1) : 0,
			(flags & 2) ? (poly->size.y() - 1) : 0,
			(flags & 4) ? (poly->size.z() - 1) : 0);
		ret.origin = currentOrigin + poly->lbound; //new origin

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			ret.size[i] = poly->size[idx]; //new size of dimension i

			ret.delta[i] = osg::Vec3i();
			ret.delta[i][idx] = ((flags >> idx) & 1) ? -1 : 1; //new delta
		}
	}

	void PolyhedronPosition::applyTransform(const Polyhedron* poly, osg::Matrix& ret, bool useMapPosition) const {
		ret.postMultTranslate(osg::Vec3(
			(flags & 1) ? (poly->lbound.x() - poly->size.x()) : poly->lbound.x(),
			(flags & 2) ? (poly->lbound.y() - poly->size.y()) : poly->lbound.y(),
			(flags & 4) ? (poly->lbound.z() - poly->size.z()) : poly->lbound.z()));

		ret.postMultScale(osg::Vec3(
			(flags & 1) ? -1 : 1,
			(flags & 2) ? -1 : 1,
			(flags & 4) ? -1 : 1));

		osg::Matrix tmp = ret;

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			for (int j = 0; j < 4; j++) {
				ret(j, i) = tmp(j, idx);
			}
		}
		for (int j = 0; j < 4; j++) {
			ret(j, 3) = tmp(j, 3);
		}

		if (useMapPosition) MapPosition::applyTransform(ret);
	}

	void PolyhedronPosition::getTransformAnimation(const Polyhedron* poly, MoveDirection dir, osg::Matrix& mat1, osg::Quat& quat, osg::Matrix& mat2, bool useMapPosition) const {
		mat1.postMultTranslate(osg::Vec3(
			(flags & 1) ? (poly->lbound.x() - poly->size.x()) : poly->lbound.x(),
			(flags & 2) ? (poly->lbound.y() - poly->size.y()) : poly->lbound.y(),
			(flags & 4) ? (poly->lbound.z() - poly->size.z()) : poly->lbound.z()));

		mat1.postMultScale(osg::Vec3(
			(flags & 1) ? -1 : 1,
			(flags & 2) ? -1 : 1,
			(flags & 4) ? -1 : 1));

		osg::Matrix tmp = mat1;
		osg::Vec3i size;

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			size[i] = poly->size[idx]; //new size of dimension i
			for (int j = 0; j < 4; j++) {
				mat1(j, i) = tmp(j, idx);
			}
		}
		for (int j = 0; j < 4; j++) {
			mat1(j, 3) = tmp(j, 3);
		}

		osg::Vec3 offset;
		switch (dir) {
		case MOVE_NEG_X:
			quat.makeRotate(osg::Vec3(0, 0, 1), osg::Vec3(-1, 0, 0));
			break;
		case MOVE_POS_X:
			offset.x() = size.x();
			quat.makeRotate(osg::Vec3(0, 0, 1), osg::Vec3(1, 0, 0));
			break;
		case MOVE_NEG_Y:
			quat.makeRotate(osg::Vec3(0, 0, 1), osg::Vec3(0, -1, 0));
			break;
		case MOVE_POS_Y:
			offset.y() = size.y();
			quat.makeRotate(osg::Vec3(0, 0, 1), osg::Vec3(0, 1, 0));
			break;
		}

		mat1.postMultTranslate(-offset);

		mat2.makeTranslate(offset);
		if (useMapPosition) MapPosition::applyTransform(mat2);
	}


	void PolyhedronPosition::move(const Polyhedron* poly, MoveDirection dir){
		//get current size
		osg::Vec3i size, oflags, nflags;
		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			oflags[i] = idx;
			size[i] = poly->size[idx]; //new size of dimension i
		}

#define NFLAGS(X,Y,Z) nflags.x() = oflags.X(); nflags.y() = oflags.Y(); nflags.z() = oflags.Z()
#define NORIGIN(X) flags ^= (1 << oflags.X())

		switch (dir) {
		case MOVE_NEG_X:
			pos.x() -= size.z();
			NORIGIN(z);
			NFLAGS(z, y, x);
			break;
		case MOVE_POS_X:
			pos.x() += size.x();
			NORIGIN(x);
			NFLAGS(z, y, x);
			break;
		case MOVE_NEG_Y:
			pos.y() -= size.z();
			NORIGIN(z);
			NFLAGS(x, z, y);
			break;
		case MOVE_POS_Y:
			pos.y() += size.y();
			NORIGIN(y);
			NFLAGS(x, z, y);
			break;
		default:
			return;
		}

#undef NFLAGS
#undef NORIGIN

		flags = (flags & UPPER_MASK) | (nflags.x() << 3) | (nflags.y() << 5) | (nflags.z() << 7);
	}

	bool PolyhedronPosition::load(const std::string& data, Level* parent, MapData* mapData){
		/* format is
		<MapPosition>[:...[:...[:...]]] <-- this is currently unsupported
		*/

		if (!MapPosition::load(data, parent, mapData)) return false;

		//TODO: load flags
		flags = ROT_XYZ;

		return true;
	}

	osgDB::InputStream& operator>>(osgDB::InputStream& s, PolyhedronPosition& obj){
		s >> (MapPosition&)obj >> obj.flags;
		return s;
	}

	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const PolyhedronPosition& obj){
		s << (const MapPosition&)obj << obj.flags;
		return s;
	}

	class TestPolyhedronAnimator : public osgGA::GUIEventHandler {
	public:
		TestPolyhedronAnimator(Polyhedron* poly, MoveDirection dir, int moveType)
			: _poly(poly)
			, _t(0)
			, _moveType(moveType)
		{
			if (moveType & Polyhedron::ROLLING_ALL) {
				poly->pos.getTransformAnimation(poly, MoveDirection(dir ^ 1), _mat1, _quat, _mat2);
			} else if (moveType & Polyhedron::MOVING_ALL) {
				PolyhedronPosition oldPos = poly->pos;
				oldPos.applyTransform(poly, _mat2);
				switch (dir) {
				case MOVE_NEG_X: oldPos.pos.x()++; break;
				case MOVE_POS_X: oldPos.pos.x()--; break;
				case MOVE_NEG_Y: oldPos.pos.y()++; break;
				case MOVE_POS_Y: oldPos.pos.y()--; break;
				case MOVE_NEG_Z: oldPos.pos.z()++; break;
				case MOVE_POS_Z: oldPos.pos.z()--; break;
				}
				oldPos.applyTransform(poly, _mat1);
				for (int i = 0; i < 16; i++) {
					_mat2.ptr()[i] -= _mat1.ptr()[i];
				}
			}
		}

		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
			//TODO: remove event callback after animation finished
			if (_t < 8 && ea.getEventType() == osgGA::GUIEventAdapter::FRAME) {
				_t++;
				if (_t >= 8) {
					_poly->updateTransform();
				} else if (_moveType & Polyhedron::ROLLING_ALL) {
					osg::Matrix mat = _mat1;
					osg::Quat q;
					q.slerp(_t*0.125f, _quat, osg::Quat());
					mat.postMultRotate(q);
					mat.postMult(_mat2);
					_poly->_trans->setMatrix(mat);
				} else if (_moveType & Polyhedron::MOVING_ALL) {
					float t = _t*0.125f;
					osg::Matrix mat = _mat1;
					for (int i = 0; i < 16; i++) {
						mat.ptr()[i] += t*_mat2.ptr()[i];
					}
					_poly->_trans->setMatrix(mat);
				}
			}

			return false;
		}
	private:
		osg::ref_ptr<Polyhedron> _poly;
		osg::Matrix _mat1, _mat2;
		osg::Quat _quat;
		int _t;
		int _moveType;
	};

	Polyhedron::Polyhedron()
		: shape(0)
		, flags(0)
		, movement(0)
		, controller(0)
		, size(1, 1, 2)
		, customShapeEnabled(false)
		, customShape(1, SOLID)
		, _objType(NULL)
	{
	}

	Polyhedron::Polyhedron(const Polyhedron& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, id(other.id)
		, shape(other.shape)
		, objType(other.objType)
		, flags(other.flags)
		, movement(other.movement)
		, controller(other.controller)
		, pos(other.pos)
		, lbound(other.lbound)
		, size(other.size)
		, customShapeEnabled(other.customShapeEnabled)
		, customShape(other.customShape)
		, _objType(NULL)
	{

	}

	Polyhedron::~Polyhedron()
	{
	}

	unsigned char& Polyhedron::operator()(int x, int y, int z){
		int idx = customShapeEnabled ? ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x() : 0;
		return customShape[idx];
	}

	unsigned char Polyhedron::operator()(int x, int y, int z) const{
		int idx = customShapeEnabled ? ((z - lbound.z())*size.y() + y - lbound.y())*size.x() + x - lbound.x() : 0;
		return customShape[idx];
	}

	void Polyhedron::resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool customShape_, bool preserved){
		bool old = customShapeEnabled;
		customShapeEnabled = customShape_;

		if (!(preserved && customShape_ && old)) {
			size = size_;

			unsigned char c = preserved ? customShape[0] : SOLID;
			if (c == 0) c = 1;
			customShape.resize(customShape_ ? size_.x()*size_.y()*size_.z() : 1, c);

			return;
		}

		std::vector<unsigned char> tmp = customShape;
		customShape.resize(size_.x()*size_.y()*size_.z(), 0);

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
					customShape[new_idx] = tmp[old_idx];
				}
			}
		}

		lbound = lbound_;
		size = size_;
	}

	void Polyhedron::createInstance(){
		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		
		switch (shape) {
		case CUBOID:
			if (customShapeEnabled) {
				int idx = 0;

				for (int z = lbound.z(); z < lbound.z() + size.z(); z++) {
					for (int y = lbound.y(); y < lbound.y() + size.y(); y++) {
						for (int x = lbound.x(); x < lbound.x() + size.x(); x++) {
							unsigned char c = customShape[idx];

							//TODO: block type
							switch (c) {
							case SOLID:
								//test only
								geode->addDrawable(gfx::createCube(
									osg::Vec3(x, y, z),
									osg::Vec3(x + 1, y + 1, z + 1),
									false,
									0.05f,
									osg::Vec3(0.3f, 0.3f, 0.3f)
									));
								break;
							}

							idx++;
						}
					}
				}
			} else {
				unsigned char c = customShape[0];

				//TODO: block type
				switch (c) {
				case SOLID:
					//test only
					geode->addDrawable(gfx::createCube(
						osg::Vec3(lbound.x(), lbound.y(), lbound.z()),
						osg::Vec3(lbound.x() + size.x(), lbound.y() + size.y(), lbound.z() + size.z()),
						false,
						0.05f,
						osg::Vec3(0.3f, 0.3f, 0.3f)
						));
					break;
				}
			}
			break;
		}

		_appearance = geode;

		_trans = new osg::MatrixTransform;
		_trans->addChild(geode.get());
	}

	void Polyhedron::updateTransform(){
		if (!_trans.valid() || pos._map == NULL) return;

		osg::Matrix mat;

		//ad-hoc get transformation matrix
		pos.applyTransform(this, mat);

		_trans->setMatrix(mat);
	}

	bool Polyhedron::move(MoveDirection dir){
		if (!_trans.valid() || pos._map == NULL) return false;

		PolyhedronPosition newPos = pos;
		int moveType = 0;

		//TODO: rotating block
		switch (dir) {
		case MOVE_NEG_X:
		case MOVE_POS_X:
			if (movement & ROLLING_X) {
				moveType = ROLLING_X;
				newPos.move(this, dir);
			} else if (movement & MOVING_X) {
				moveType = MOVING_X;
				newPos.pos.x() += (dir == MOVE_POS_X) ? 1 : -1;
			} else {
				return false;
			}
			break;
		case MOVE_NEG_Y:
		case MOVE_POS_Y:
			if (movement & ROLLING_Y) {
				moveType = ROLLING_Y;
				newPos.move(this, dir);
			} else if (movement & MOVING_Y) {
				moveType = MOVING_Y;
				newPos.pos.y() += (dir == MOVE_POS_Y) ? 1 : -1;
			} else {
				return false;
			}
			break;
		case MOVE_NEG_Z:
		case MOVE_POS_Z:
			if (movement & MOVING_Z) {
				moveType = MOVING_Z;
				newPos.pos.z() += (dir == MOVE_POS_Z) ? 1 : -1;
			} else {
				return false;
			}
			break;
		default:
			return false;
		}

		//check if it hits something during rolling
		if ((moveType & ROLLING_ALL) && (flags & CONTINUOUS_HITTEST)) {
			if (!isRollable(dir)) return false;
		}

		//check if the end position is valid
		if (valid(newPos)) {
			pos = newPos;
			_trans->setEventCallback(new TestPolyhedronAnimator(this, dir, moveType));
			return true;
		}

		return false;
	}

	bool Polyhedron::isRollable(const PolyhedronPosition& pos, MoveDirection dir) const {
		//get move type
		const bool isNegative = (dir == MOVE_NEG_X || dir == MOVE_NEG_Y);
		const bool isY = (dir == MOVE_NEG_Y || dir == MOVE_POS_Y);

		//get current position
		PolyhedronPosition::Idx iii;
		pos.getCurrentPos(this, iii);

		//swap size if move is along Y axis
		if (isY) {
			std::swap(iii.size.x(), iii.size.y());
			std::swap(iii.delta.x(), iii.delta.y());
		}

		const int sz = pos._map->lbound.z();
		const int ez = pos._map->lbound.z() + pos._map->size.z();

		//create a 2D array to save hit test area
		const int w = iii.size.x() + iii.size.z();
		const int h = floorf(sqrtf(iii.size.x()*iii.size.x() + iii.size.z()*iii.size.z()) + 2.0f);
		const int offset = isNegative ? -iii.size.z() : 0;
		const int size = w*h;
		std::vector<char> hitTestArea(size);

		//y coord if move along X axis, otherwise it is x coord
		for (int y = 0; y < iii.size.y(); y++) {
			//calculate hit test area
			int idx = iii.origin;
			memset(&(hitTestArea[0]), 0, size);

			//x coord if move along X axis, otherwise it is y coord
			for (int x = 0; x < iii.size.x(); x++) {
				for (int z = 0; z < iii.size.z(); z++) {
					if (customShape[customShapeEnabled ? (idx + z*iii.delta.z()) : 0]) {
						//it is non-empty, calculate the hit test area
						int x1 = isNegative ? (-x - 1) : (x - iii.size.x());

						const int v0x = x1 * 2 + 1, v0y = z * 2 + 1; //old vector, x coord should be negative
						const int d0 = v0x*v0x + v0y*v0y; //4*square(length(v0))
						const int d0_minus_1 = d0 - 4 * (v0y - v0x) + 4; //not larger than 4*square(length(v0)-1)

						for (; x1 <= z; x1++) {
							const int v1x = x1 * 2 + 1;
							const int v1x_sq = v1x*v1x;

							//calculate the min z
							int z1 = 0;
							if (v1x_sq < d0_minus_1) {
								z1 = floorf(sqrtf(d0_minus_1 - v1x_sq)*0.5f);
							}

							for (; z1 < h; z1++) {
								const int v1y = z1 * 2 + 1;

								//check if the angle is between 0 and 90
								if (v0x*v1y - v0y*v1x > 0) continue; //angle < 0
								if (v0x*v1x + v0y*v1y < 0) continue; //angle > 90

								const int d1 = v1x_sq + v1y*v1y; //4*square(length(v1))

								//check if abs(length(v1)-length(v0))<1. (simplified using middle school mathematics)
								const int tmp = ((d1 - d0) >> 2) - 1;
								if (tmp*tmp < d0) {
									//it is in the hit test area
									if (isNegative) {
										hitTestArea[z1*w - x1 - 1 + iii.size.z()] = 1;
									} else {
										hitTestArea[z1*w + x1 + iii.size.x()] = 1;
									}
								} else if (tmp > 0) {
									//it is too long
									break;
								}
							}
						}
					}
				}
				idx += iii.delta.x();
			}

#if 0
			//DEBUG
			UTIL_INFO "Hit test area:" << std::endl;
			for (int z = h - 1; z >= 0; z--) {
				for (int x = 0; x < w; x++) {
					OSG_NOTICE << int(hitTestArea[z*w + x]);
				}
				OSG_NOTICE << std::endl;
			}
#endif

			//now do the actual hit test

			//x coord if move along X axis, otherwise it is y coord
			for (int x = 0; x < w; x++) {
				const int xx = pos.pos.x() + (isY ? y : (x + offset));
				const int yy = pos.pos.y() + (isY ? (x + offset) : y);

				for (int zz = sz; zz < ez; zz++) {
					TileType* t = pos._map->get(xx, yy, zz);
					if (t) {
						//calculate the range
						int z = zz + t->blockedArea[0] - pos.pos.z();
						int e = zz + t->blockedArea[1] - pos.pos.z();

						//check if some blocks is blocked
						if (z < 0) z = 0;
						if (e > h) e = h;
						for (; z < e; z++) {
							if (hitTestArea[z*w + x]) {
								//it is blocked
								return false;
							}
						}
					}
				}
			}

			iii.origin += iii.delta.y();
		}

		return true;
	}

	bool Polyhedron::valid(const PolyhedronPosition& pos) const {
		//get current position
		PolyhedronPosition::Idx iii;
		pos.getCurrentPos(this, iii);

		//used if stable mode is SPANNABLE
		util::Rect<int> r(iii.size.x() + 1, iii.size.y() + 1, -1, -1);

		//used if stable mode is 0 or PARTIAL_FLOATING
		int suppCount = 0, blockCount = 0;

		const unsigned char c = customShapeEnabled ? 0 : customShape[0];
		bool isEmpty = c == 0; //if the buffer is all 0
		std::vector<unsigned char> zBlocks(iii.size.z(), c); //a temp buffer for custom shapes

		//check blocks of polyhedron
		int yy = pos.pos.y(); //xx,yy,zz=coord in map space
		const int sz = pos._map->lbound.z();
		const int ez = pos._map->lbound.z() + pos._map->size.z();

		//x,y=coord in polyhedron space
		for (int y = 0; y < iii.size.y(); y++) {
			int idx = iii.origin;
			int xx = pos.pos.x();

			for (int x = 0; x < iii.size.x(); x++) {
				//we need to reload blocks only if custom shape is enabled
				if (customShapeEnabled) {
					isEmpty = true;
					for (int z = 0; z < iii.size.z(); z++) {
						unsigned char c = customShape[idx + z*iii.delta.z()];
						if (c) isEmpty = false;
						zBlocks[z] = c;
					}
				}

				if (!isEmpty) {
					bool needSupport = zBlocks[0] == SOLID;
					bool supported = false;

					for (int zz = sz; zz < ez; zz++) {
						TileType* t = pos._map->get(xx, yy, zz);
						if (t) {
							//calculate the range
							int z = zz + t->blockedArea[0] - pos.pos.z();
							int e = zz + t->blockedArea[1] - pos.pos.z();

							//check if the bottom block is supported
							if (needSupport && (t->flags & TileType::SUPPORTER) && e == 0) {
								//it is supported
								supported = true;
							}

							//check if some blocks is blocked
							if (z < 0) z = 0;
							if (e > iii.size.z()) e = iii.size.z();
							for (; z < e; z++) {
								if (zBlocks[z]) {
									//it is blocked
									return false;
								}
							}
						}
					}

					if (needSupport) {
						blockCount++; //count solid blocks
						if (supported) {
							suppCount++; //count supported blocks
							r.expandBy(x, y); //expand bounding box
						}
					}
				}

				idx += iii.delta.x();
				xx++;
			}

			iii.origin += iii.delta.y();
			yy++;
		}

		//check if it is stable
		if (flags & FLOATING) {
			return true;
		} else if (flags & SPANNABLE) {
			if (r.left <= 0 && r.top <= 0 && r.right >= iii.size.x() - 1 && r.bottom >= iii.size.y() - 1) {
				return true;
			}
		} else if (flags & PARTIAL_FLOATING) {
			if (suppCount >= 1) return true;
		} else {
			if (suppCount >= blockCount) return true;
		}

		return false;
	}

	void Polyhedron::init(Level* parent){
		_objType = parent->objectTypeMap->lookup(objType);
		pos.init(parent);

		//check size
		{
			size_t n = customShapeEnabled ? size.x()*size.y()*size.z() : 1;
			size_t m = customShape.size();
			if (m < n) {
				UTIL_WARN "data size mismatch, expected: " << n << ", actual: " << m << std::endl;
				customShape.reserve(n);
				for (; m < n; m++) customShape.push_back(SOLID);
			}
		}
	}

	bool Polyhedron::load(const XMLNode* node, Level* parent, MapData* mapData){
		id = node->getAttr("id", std::string());

		//get shape
		{
			std::string s = node->getAttr("shape", std::string("cuboid"));
			bool isVec3i = true;
			for (size_t i = 0; i < s.size(); i++) {
				char c = s[i];
				if ((c >= '0' && c <= '9') || c == ',') {
					//do nothing
				} else if (c == 'X' || c == 'x') {
					s[i] = ',';
				} else {
					isVec3i = false;
					break;
				}
			}

			if (isVec3i) {
				shape = CUBOID;
				size = util::getAttrFromStringOsgVec(s, osg::Vec3i(1, 1, 1));
			} else if (s == "cube") {
				shape = CUBOID;
				size.set(1, 1, 1);
			} else if (s == "cuboid" || s == "classical") {
				shape = CUBOID;
				size.set(1, 1, 2);
			} else {
				UTIL_WARN "unrecognized shape: '" << s << "'" << std::endl;
				return false;
			}
		}

		if (size.x() <= 0 || size.y() <= 0 || size.z() <= 0) {
			UTIL_WARN "invalid size" << std::endl;
			return false;
		}

		const int theSize = size.x()*size.y()*size.z();

		customShapeEnabled = false;
		customShape.resize(1, SOLID);

		//lbound
		lbound = node->getAttrOsgVec("lbound", osg::Vec3i());

		//get position
		pos.load(node->getAttr("p", std::string()), parent, mapData);

		//object type
		objType = node->getAttr("type", std::string());

		//flags
#define GETFLAGS(NAME,DEFAULT,FLAGS) (node->getAttr(NAME, DEFAULT) ? FLAGS : 0)
		flags = GETFLAGS("discardable", false, DISCARDABLE)
			| GETFLAGS("main", true, MAIN)
			| GETFLAGS("fragile", true, FRAGILE)
			| GETFLAGS("partialFloating", false, PARTIAL_FLOATING)
			| GETFLAGS("supporter", true, SUPPORTER)
			| GETFLAGS("tiltable", true, TILTABLE)
			| GETFLAGS("tilt-supporter", true, TILT_SUPPORTER)
			| GETFLAGS("spannable", true, SPANNABLE)
			| GETFLAGS("visible", true, VISIBLE)
			| GETFLAGS("floating", false, FLOATING)
			| GETFLAGS("targetBlock", false, TARGET)
			| GETFLAGS("exitBlock", false, EXIT);
		flags |= GETFLAGS("continuousHittest", (flags & TILTABLE) ? true : false, CONTINUOUS_HITTEST);
#undef GETFLAGS

		//controller
		{
			std::string s = node->getAttr("controller", std::string("player"));
			if (s == "player") controller = PLAYER;
			else if (s == "elevator") controller = ELEVATOR;
			else controller = PASSIVE;
		}

		//movement
		movement = node->getAttr("movement", ROLLING_ALL);

		//load subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "customShape") {
				//this node contains an array
				/*
				format: <value>["*"<count>]
				","=next pos (x++)
				";"=next row (y++)
				"|"=next plane (z++)
				*/
				customShapeEnabled = true;
				customShape.resize(theSize, 0);

				const std::string& contents = subnode->contents;
				std::string::size_type lps = 0;
#define GET_CHARACTER() util::getCharacter(contents, lps)

				int c = 0;
				osg::Vec3i p;
				int type, count;

				for (;;) {
					//get type
					std::string s;
					for (;;) {
						c = GET_CHARACTER();
						if (c == '*' || c == ',' || c == ';' || c == '|' || c == EOF) break;
						s.push_back(c);
					}
					if (!s.empty()) type = atoi(s.c_str());
					else type = 0;

					//get count
					count = 1;
					if (c == '*') {
						s.clear();
						for (;;) {
							c = GET_CHARACTER();
							if (c == ',' || c == ';' || c == '|' || c == EOF) break;
							s.push_back(c);
						}
						if (!s.empty()) count = atoi(s.c_str());
					}

					//put these data to array, and advance to next position
					int idx = (p.z()*size.y() + p.y())*size.x() + p.x();
					for (int i = 0; i < count; i++) {
						if (idx < theSize) {
							customShape[idx] = type;
						}
						idx++;

						util::typeArrayAdvance(p, size, i == count - 1, c);
					}

					if (c == EOF) break;
					if (c != ',' && c != ';' && c != '|') {
						UTIL_WARN "unexpected character: '" << char(c) << "', expected ',' or ';' or '|'" << std::endl;
						break;
					}
				}
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	REG_OBJ_WRAPPER(game, Polyhedron, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(shape, 0);
		ADD_STRING_SERIALIZER(objType, "");
		ADD_INT_SERIALIZER(flags, 0);
		ADD_INT_SERIALIZER(movement, 0);
		ADD_INT_SERIALIZER(controller, 0);
		ADD_REF_ANY_SERIALIZER(pos, PolyhedronPosition, PolyhedronPosition());
		ADD_VEC3I_SERIALIZER(lbound, osg::Vec3i());
		ADD_VEC3I_SERIALIZER(size, osg::Vec3i(1, 1, 2));
		ADD_BOOL_SERIALIZER(customShapeEnabled, false);
		ADD_VECTOR_SERIALIZER(customShape, std::vector<unsigned char>, osgDB::BaseSerializer::RW_UCHAR, -1);
	}

}
