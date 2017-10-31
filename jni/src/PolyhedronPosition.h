#pragma once

#include <osg/Matrix>
#include <osg/Quat>
#include <osg/Vec3i>
#include <string>
#include "MapPosition.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

class XMLNode;

namespace game {

	class Level;
	class MapData;
	class Polyhedron;

	/** Represents position of a polyhedron. */

	class PolyhedronPosition : public MapPosition {
	public:
		/// The flags of \ref flags.
		enum Flags {
			UPPER_X = 1, //!< mirrored
			UPPER_Y = 2, //!< mirrored
			UPPER_Z = 4, //!< mirrored
			UPPER_XY = 1 | 2,
			UPPER_XZ = 1 | 4,
			UPPER_YZ = 2 | 4,
			UPPER_MASK = 1 | 2 | 4,
			ROT_XYZ = (0 << 3) | (1 << 5) | (2 << 7),
			ROT_YZX = (1 << 3) | (2 << 5) | (0 << 7),
			ROT_ZXY = (2 << 3) | (0 << 5) | (1 << 7),
			ROT_XZY = (0 << 3) | (2 << 5) | (1 << 7), //!< mirrored
			ROT_YXZ = (1 << 3) | (0 << 5) | (2 << 7), //!< mirrored
			ROT_ZYX = (2 << 3) | (1 << 5) | (0 << 7), //!< mirrored
			ROT_MASK = (3 << 3) | (3 << 5) | (3 << 7),
		};
		/// used in \ref getCurrentPos.
		struct Pos {
			osg::Vec3i origin; //!< current origin
			osg::Vec3i size; //!< current size
			osg::Vec3i delta[3]; //!< current delta x,y,z
		};
		/// used in \ref getCurrentPos.
		struct Idx {
			int origin; //!< index of current origin
			osg::Vec3i size; //!< current size
			osg::Vec3i delta; //!< current delta x,y,z
		};
	public:
		PolyhedronPosition() :
			flags(ROT_XYZ)
		{
		}
		///ad-hoc, don't use
		bool operator!=(const PolyhedronPosition& other) const {
			return map != other.map || pos != other.pos || flags != other.flags;
		}
		void init(Level* parent);

		/** Get current position.
		\param[in] flags The \ref flags.
		\param[in] poly The polyhedron (should be cuboid).
		\param[out] ret The result
		*/
		static void getCurrentPos(int flags, const Polyhedron* poly, Pos& ret);
		void getCurrentPos(const Polyhedron* poly, Pos& ret) const {
			return getCurrentPos(flags, poly, ret);
		}

		/** Get current position.
		\param[in] flags The \ref flags.
		\param[in] poly The polyhedron (should be cuboid).
		\param[out] ret The result
		*/
		static void getCurrentPos(int flags, const Polyhedron* poly, Idx& ret);
		void getCurrentPos(const Polyhedron* poly, Idx& ret) const {
			return getCurrentPos(flags, poly, ret);
		}

		/** apply transform to the specified matrix.
		\param[in] poly The polyhedron (should be cuboid).
		\param[in,out] ret The matrix.
		\param[in] useMapPosition use map position, otherwise the polyhedron is at origin.
		*/
		void applyTransform(const Polyhedron* poly, osg::Matrix& ret, bool useMapPosition = true) const;

		/** get the transform of polyhedron animation.
		The transform matrix is `mat1*slerp(1,quat,t)*mat2`.
		\param[in] poly The polyhedron (should be cuboid).
		\param[in] dir The direction.
		\param[in,out] mat1 The matrix.
		\param[out] quat The rotation.
		\param[out] mat2 The matrix.
		\param[in] useMapPosition use map position, otherwise the polyhedron is at origin.
		*/
		void getTransformAnimation(const Polyhedron* poly, MoveDirection dir, osg::Matrix& mat1, osg::Quat& quat, osg::Matrix& mat2, bool useMapPosition = true) const;

		///move to the adjacent position (no sanity check)
		void move(const Polyhedron* poly, MoveDirection dir);

		bool load(const std::string& data, Level* parent, MapData* mapData); //!< load from a string in XML node,
	public:
		/** Flags.
		* * If the shape is cuboid:
		*   * bit 0-2: determines the current origin of polyhedron (x,y,z, 0=lower, 1=upper)
		*   * bit 3-4,5-6,7-8: determines the current delta x,y,z of polyhedron (thus determines the new size)
		* * Otherwise:
		*   * Currently unsupported.
		*/
		int flags;
	};

	osgDB::InputStream& operator>>(osgDB::InputStream& s, PolyhedronPosition& obj);
	osgDB::OutputStream& operator<<(osgDB::OutputStream& s, const PolyhedronPosition& obj);

}
