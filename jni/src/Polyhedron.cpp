#include "Polyhedron.h"
#include "ObjectType.h"
#include "Level.h"
#include "SimpleGeometry.h"
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>
#include <assert.h>

namespace game {

	void PolyhedronPosition::init(Level* parent){
		MapPosition::init(parent);
	}

	void PolyhedronPosition::getCurrentPos(int flags, const Polyhedron* poly, int ret[7]) {
		osg::Vec3i currentOrigin(
			(flags & 1) ? (poly->size.x() - 1) : 0,
			(flags & 2) ? (poly->size.y() - 1) : 0,
			(flags & 4) ? (poly->size.z() - 1) : 0);
		ret[0] = (currentOrigin.z()*poly->size.y() + currentOrigin.y())*poly->size.x() + currentOrigin.x(); //new origin

		assert(((1 << ((flags >> 3) & 3)) | (1 << ((flags >> 5) & 3)) | (1 << ((flags >> 7) & 3))) == 7);

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			ret[1 + i] = poly->size[idx]; //new size of dimension i

			int delta = ((flags >> i) & 1) ? -1 : 1;
			if (idx >= 1) delta *= poly->size.x();
			if (idx >= 2) delta *= poly->size.y();
			ret[4 + i] = delta; //new delta
		}
	}

	void PolyhedronPosition::getCurrentPos(int flags, const Polyhedron* poly, osg::Vec3i ret[5]) {
		osg::Vec3i currentOrigin(
			(flags & 1) ? (poly->size.x() - 1) : 0,
			(flags & 2) ? (poly->size.y() - 1) : 0,
			(flags & 4) ? (poly->size.z() - 1) : 0);
		ret[0] = currentOrigin + poly->lbound; //new origin

		assert(((1 << ((flags >> 3) & 3)) | (1 << ((flags >> 5) & 3)) | (1 << ((flags >> 7) & 3))) == 7);

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			ret[1][i] = poly->size[idx]; //new size of dimension i

			ret[2 + i] = osg::Vec3i();
			ret[2 + i][idx] = ((flags >> i) & 1) ? -1 : 1; //new delta
		}
	}

	void PolyhedronPosition::applyTransform(const Polyhedron* poly, osg::Matrix& ret) const {
		ret.postMultTranslate(osg::Vec3(
			(flags & 1) ? (poly->lbound.x() - poly->size.x()) : poly->lbound.x(),
			(flags & 2) ? (poly->lbound.y() - poly->size.y()) : poly->lbound.y(),
			(flags & 4) ? (poly->lbound.z() - poly->size.z()) : poly->lbound.z()));

		ret.postMultScale(osg::Vec3(
			(flags & 1) ? -1 : 1,
			(flags & 2) ? -1 : 1,
			(flags & 4) ? -1 : 1));

		assert(((1 << ((flags >> 3) & 3)) | (1 << ((flags >> 5) & 3)) | (1 << ((flags >> 7) & 3))) == 7);

		osg::Matrix mat = ret;

		for (int i = 0; i < 3; i++) {
			int idx = (flags >> (3 + i * 2)) & 3; //new index, should be 0,1,2
			for (int j = 0; j < 4; j++) {
				ret(j, i) = mat(j, idx);
			}
		}
		for (int j = 0; j < 4; j++) {
			ret(j, 3) = mat(j, 3);
		}

		MapPosition::applyTransform(ret);
	}

	osgDB::InputStream& operator>>(osgDB::InputStream& s, PolyhedronPosition& obj){
		s >> (MapPosition&)obj >> obj.flags;
		return s;
	}

	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const PolyhedronPosition& obj){
		s << (const MapPosition&)obj << obj.flags;
		return s;
	}

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
								geode->addDrawable(geom::createCube(
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
					geode->addDrawable(geom::createCube(
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

	void Polyhedron::updateTransform(Level* parent){
		if (!_trans.valid() || pos._map == NULL) return;

		osg::Matrix mat;

		//ad-hoc get transformation matrix
		pos.applyTransform(this, mat);

		_trans->setMatrix(mat);
	}

	void Polyhedron::init(Level* parent){
		_objType = parent->objectTypeMap->lookup(objType);
		pos.init(parent);

		//check size
		{
			size_t n = customShapeEnabled ? size.x()*size.y()*size.z() : 1;
			size_t m = customShape.size();
			if (m < n) {
				OSG_NOTICE << "[" __FUNCTION__ "] data size mismatch, expected: " << n << ", actual: " << m << std::endl;
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
		ADD_VECTOR_SERIALIZER(customShape, std::vector<unsigned char>, osgDB::BaseSerializer::RW_UCHAR, 32);
	}

}
