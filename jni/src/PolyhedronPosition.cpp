#include "PolyhedronPosition.h"
#include "Level.h"
#include "Polyhedron.h"
#include "util_err.h"
#include "util_misc.h"
#include "XMLReaderWriter.h"
#include <osgDB/ObjectWrapper>

namespace game {

	const int PolyhedronPosition::allPossibleFlagsForCuboidSize[numberOfAllPossibleFlagsForCuboidSize] =
	{
		ROT_XYZ,
		ROT_YZX,
		ROT_ZXY,
		ROT_XZY | UPPER_X,
		ROT_YXZ | UPPER_X,
		ROT_ZYX | UPPER_X,
	};
	const int PolyhedronPosition::allPossibleFlagsForCuboid[numberOfAllPossibleFlagsForCuboid] =
	{
		ROT_XYZ,
		ROT_XYZ | UPPER_XY,
		ROT_XYZ | UPPER_XZ,
		ROT_XYZ | UPPER_YZ,
		ROT_YZX,
		ROT_YZX | UPPER_XY,
		ROT_YZX | UPPER_XZ,
		ROT_YZX | UPPER_YZ,
		ROT_ZXY,
		ROT_ZXY | UPPER_XY,
		ROT_ZXY | UPPER_XZ,
		ROT_ZXY | UPPER_YZ,
		ROT_XZY | UPPER_X,
		ROT_XZY | UPPER_Y,
		ROT_XZY | UPPER_Z,
		ROT_XZY | UPPER_XYZ,
		ROT_YXZ | UPPER_X,
		ROT_YXZ | UPPER_Y,
		ROT_YXZ | UPPER_Z,
		ROT_YXZ | UPPER_XYZ,
		ROT_ZYX | UPPER_X,
		ROT_ZYX | UPPER_Y,
		ROT_ZYX | UPPER_Z,
		ROT_ZYX | UPPER_XYZ,
	};

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

}
