#pragma once

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Vec3i>
#include <string>
#include "Appearance.h"
#include "MapData.h"
#include "util_object.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

class XMLNode;

namespace game {

	class Level;
	class ObjectType;
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

	/// an internal class to represent a polyhedron animation.

	class PolyhedronAnimation : public osg::Referenced {
	public:
		/// the animation type
		enum AnimationType {
			ROLLING,
			MOVING,
			FLASHING,
		};
	protected:
		virtual ~PolyhedronAnimation();
	public:
		PolyhedronAnimation(Polyhedron* poly, MoveDirection dir, AnimationType type);
		bool update(); //!< update animation
	public:
		Polyhedron* _poly; //!< the polyhedron
		osg::Matrix _mat1, _mat2; //!< the matrices used if \ref ROLLING or \ref MOVING
		osg::Quat _quat; //!< the rotation used if \ref ROLLING
		int _t; //!< the animation time
		int _maxt; //!< the max animation time
		AnimationType _type; //!< the \ref AnimationType
	};

	/** Represents a polyhedron.
	* A polyhedron has the following properties:
	* * Shape:
	*   * cuboid (with size and custom shape)
	*   * a list of predefined regular polyhedra
	* * Movement:
	*   * rolling (e.g. boxes in Rolling Block Mazes/Bloxorz)
	*   * moving (e.g. boxes in Sokoban/PuzzleBoy)
	*   * rotating (e.g. rotating blocks in PuzzleBoy)
	* * Controller:
	*   * none (i.e. can only be pushed by player)
	*   * player (i.e. can only be controlled by player)
	*/

	class Polyhedron :
		public osg::Object
	{
	public:
		/// polyhedron shape
		enum PolyhedronShape {
			CUBOID,
		};
		/** polyhedron flags.
		* \note
		* * PARTIAL_FLOATING, SPANNABLE, FLOATING are mutually exclusive.
		*/
		enum PolyhedronFlags {
			DISCARDABLE = 0x1, //!< It can be killed without losing the game.
			MAIN = 0x2, //!< Main block, used in winning condition (usually game wins when all main blocks go to the exit), and game logic (only main blocks can go to the exit and eat checkpoints).
			FRAGILE = 0x4, //!< It will be killed when falling (in other words it can't fall).
			PARTIAL_FLOATING = 0x8, //!< It is stable if any one of its block is supported (e.g. blocks in PuzzleBoy).
			SUPPORTER = 0x10, //!< It can support other polyhedra.
			TILTABLE = 0x20, //!< It can be tilted (experimental, buggy).
			TILT_SUPPORTER = 0x40, //!< It can support other tilting polyhedra.
			SPANNABLE = 0x80, //!< It can be spanned, e.g. for a 1x3 block, 101 is allowed. Without this flag only 111 is allowrd. Technically, the bounding box will be considered instead of individual position.
			VISIBLE = 0x100, //!< It is visible and take part in the game logic (only an ad-hoc solution).
			FLOATING = 0x200, //!< It isn't affected by gravity, e.g. the target block in PuzzleBoy.
			TARGET = 0x400, //!< It is a target block, e.g. in Sokoban.
			EXIT = 0x800, //!< It is an exit block (e.g. an exit which checks shape).
			CONTINUOUS_HITTEST = 0x1000, //!< Enables continuous hit test (see \ref isRollable).
		};
		/// polyhedron movement
		enum PolyhedronMovement {
			ROLLING_X = 0x1,
			ROLLING_Y = 0x2,
			ROLLING_ALL = ROLLING_X | ROLLING_Y,
			MOVING_X = 0x4,
			MOVING_Y = 0x8,
			MOVING_Z = 0x10,
			MOVING_XY = MOVING_X | MOVING_Y,
			MOVING_XZ = MOVING_X | MOVING_Z,
			MOVING_YZ = MOVING_Y | MOVING_Z,
			MOVING_ALL = MOVING_X | MOVING_Y | MOVING_Z,
			ROTATING_X = 0x20,
			ROTATING_Y = 0x40,
			ROTATING_Z = 0x80,
			ROTATING_XY = ROTATING_X | ROTATING_Y,
			ROTATING_XZ = ROTATING_X | ROTATING_Z,
			ROTATING_YZ = ROTATING_Y | ROTATING_Z,
			ROTATING_ALL = ROTATING_X | ROTATING_Y | ROTATING_Z,
		};
		/// polyhedron controller
		enum PolyhedronController {
			PASSIVE, //!< It can only be pushed by player
			PLAYER, //!< It can only be controlled by player
			ELEVATOR, //!< It can only be controlled when player is standing on it
		};
		/// used in \ref customShape
		enum PolyhedronCell {
			EMPTY, //!< empty
			SOLID, //!< solid block
			ANTENNA, //!< antenna, don't need to be supported
		};
	protected:
		virtual ~Polyhedron();
	public:
		META_Object(game, Polyhedron);

		Polyhedron();
		Polyhedron(const Polyhedron& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

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

		///get or set \ref customShape at specified position (**without** bounds check)
		unsigned char& operator()(int x, int y, int z);

		///get or set \ref customShape at specified position (**without** bounds check)
		unsigned char& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}

		///get or set \ref customShape at specified position (**without** bounds check)
		unsigned char operator()(int x, int y, int z) const;

		///get or set \ref customShape at specified position (**without** bounds check)
		unsigned char operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		///get \ref customShape at specified position (with \ref customShapeEnabled and bounds check)
		unsigned char get(int x, int y, int z) const;

		///get \ref customShape at specified position (with \ref customShapeEnabled and bounds check)
		unsigned char get(const osg::Vec3i& p) const {
			return get(p.x(), p.y(), p.z());
		}

		///set \ref customShape at specified position (with \ref customShapeEnabled and bounds check)
		void set(int x, int y, int z, unsigned char value);

		///set \ref customShape at specified position (with \ref customShapeEnabled and bounds check)
		void set(const osg::Vec3i& p, unsigned char value) {
			set(p.x(), p.y(), p.z(), value);
		}

		void resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool customShape_, bool preserved);

		///test only
		void createInstance(bool isEditMode);

		///update transform according to \ref pos.
		void updateTransform();

		/** check if this polyhedron is supporting any other polyhedron.
		\return a polyhedron it supported, `NULL` if none.
		*/
		const Polyhedron* isSupportingOtherPolyhedron(const Level* parent) const;

		///move to the adjacent position (experimental).
		bool move(Level* parent, MoveDirection dir);

		/** do continuous collision hit test for rolling block.
		\param pos old position
		\param dir move direction
		\return `true` if it cal roll (i.e. hits nothing), `false` if hits something.
		*/
		bool isRollable(const Level* parent, const PolyhedronPosition& pos, MoveDirection dir) const;

		/** do continuous collision hit test for rolling block (at current position).
		\param dir move direction
		\return `true` if it cal roll (i.e. hits nothing), `false` if hits something.
		*/
		bool isRollable(const Level* parent, MoveDirection dir) const {
			return isRollable(parent, pos, dir);
		}

		///check if it is stable and not blocked at given position. (experimental, only works for cuboid polyhedron)
		bool valid(const Level* parent, const PolyhedronPosition& pos) const;

		///check if it is stable and not blocked at current position. (experimental, only works for cuboid polyhedron)
		bool valid(const Level* parent) const {
			return valid(parent, pos);
		}

		void init(Level* parent);

		bool load(const XMLNode* node, Level* parent, MapData* mapData); //!< load from XML node, assume the node is called `polyhedron`

		void setSelected(bool selected);

		bool update(); //!< update animation

	public:
		std::string id; //!< the polyhedron id
		int shape; //!< the polyhedron shape. \sa PolyhedronShape

		std::string objType; //!< the object type
		int flags; //!< the polyhedron flags. \sa PolyhedronFlags
		int movement; //!< the polyhedron movement. \sa PolyhedronMovement
		int controller; //!< the polyhedron controller. \sa PolyhedronController
		PolyhedronPosition pos; //!< the start position

		osg::Vec3i lbound; //!< lower bound of polyhedron data, which in turn determines the center of polyhedron
		osg::Vec3i size; //!< the size of polyhedron

		bool customShapeEnabled; //!< use custom shape. if it is true then \ref customShape is used, otherwise the polyhedron is a solid cuboid (only \ref customShape[0] is used).
		std::vector<unsigned char> customShape; //!< the custom shape. \sa PolyhedronCell

		/** The appearance map.
		Some predefined appearance id:
		- "" a fixed mesh
		- "solid" will instantiated at every solid block
		- "antenna" unimplemented...
		*/
		gfx::AppearanceMap appearanceMap;

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, shape);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, flags);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, movement);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, controller);
		UTIL_ADD_BYREF_GETTER_SETTER(PolyhedronPosition, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, lbound);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, size);
		UTIL_ADD_BYVAL_GETTER_SETTER(bool, customShapeEnabled);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<unsigned char>, customShape);
		UTIL_ADD_BYREF_GETTER_SETTER(gfx::AppearanceMap, appearanceMap);

	public:
		//the following properties don't save to file and is generated at runtime
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
		osg::ref_ptr<osg::MatrixTransform> _trans; //!< the appearance with correct transform
		ObjectType* _objType;

		std::vector<osg::ref_ptr<PolyhedronAnimation> > _animations;
		int _currentAnimation;
	};

}
