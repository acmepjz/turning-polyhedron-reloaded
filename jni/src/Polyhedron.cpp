#include "Polyhedron.h"
#include "ObjectType.h"
#include "Level.h"
#include "SimpleGeometry.h"
#include "Rect.h"
#include "util_err.h"
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgGA/GUIEventHandler>
#include <osgDB/ObjectWrapper>

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

			int delta = ((flags >> i) & 1) ? -1 : 1;
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
			ret.delta[i][idx] = ((flags >> i) & 1) ? -1 : 1; //new delta
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
		TestPolyhedronAnimator(Polyhedron* poly, MoveDirection dir)
			: _poly(poly)
			, _t(0)
		{
			poly->pos.getTransformAnimation(poly, MoveDirection(dir ^ 1), _mat1, _quat, _mat2);
		}

		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
			//TODO: remove event callback after animation finished
			if (_t < 8 && ea.getEventType() == osgGA::GUIEventAdapter::FRAME) {
				_t++;
				if (_t >= 8) {
					_poly->updateTransform();
				} else {
					osg::Matrix mat = _mat1;
					osg::Quat q;
					q.slerp(_t*0.125f, _quat, osg::Quat());
					mat.postMultRotate(q);
					mat.postMult(_mat2);
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
		newPos.move(this, dir);

		//TODO: check if it hits something during rolling, not just end state
		if (valid(newPos)) {
			pos = newPos;
			_trans->setEventCallback(new TestPolyhedronAnimator(this, dir));
			return true;
		}

		return false;
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
