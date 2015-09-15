#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>
#include "MapData.h"
#include "util.h"

namespace game {

	class ObjectType;

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
		/// polyhedron flags
		enum PolyhedronFlags {
			DISCARDABLE = 0x1, //!< It can be killed without losing the game.
			MAIN = 0x2, //!< Main block, used in winning condition (usually game wins when all main blocks go to the exit), and game logic (only main blocks can go to the exit and eat checkpoints).
			FRAGILE = 0x4, //!< It will be killed when falling (in other words it can't fall).
			SUPPORTABLE = 0x8, //!< It can be supported by other polyhedra. Without this flag it can only be supported by solid blocks in MapData.
			SUPPORTER = 0x10, //!< It can support other polyhedra.
			TILTABLE = 0x20, //!< It can be tilted (experimental, buggy).
			TILT_SUPPORTER = 0x40, //!< It can support other tilting polyhedra.
			SPANNABLE = 0x80, //!< It can be spanned, e.g. for a 1x3 block, 101 is allowed. Without this flag only 111 is allowrd. Technically, the bounding box will be considered instead of individual position.
			VISIBLE = 0x100, //!< It is visible and take part in the game logic (only an ad-hoc solution).
			FLOATING = 0x200, //!< It isn't affected by gravity, e.g. the target block in PuzzleBoy.
			TARGET = 0x400, //!< It is a target block, e.g. in Sokoban.
			EXIT = 0x800, //!< It is an exit block (will check shape).
		};
		/// polyhedron movement
		enum PolyhedronMovement {
			ROLLING_X = 0x1,
			ROLLING_Y = 0x2,
			ROLLING_ALL = ROLLING_X | ROLLING_Y,
			MOVING_X = 0x4,
			MOVING_Y = 0x8,
			MOVING_Z = 0x10,
			MOVING_ALL = MOVING_X | MOVING_Y | MOVING_Z,
			ROTATING_X = 0x20,
			ROTATING_Y = 0x40,
			ROTATING_Z = 0x80,
			ROTATING_ALL = ROTATING_X | ROTATING_Y | ROTATING_Z,
		};
		/// polyhedron controller
		enum PolyhedronController {
			NONE, //!< It can only be pushed by player
			PLAYER, //!< It can only be controlled by player
			ELEVATOR, //!< It can only be controlled when player is standing on it
		};
	protected:
		virtual ~Polyhedron();
	public:
		META_Object(game, Polyhedron);

		Polyhedron();
		Polyhedron(const Polyhedron& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		unsigned char& operator()(int x, int y, int z);
		unsigned char& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}
		unsigned char operator()(int x, int y, int z) const;
		unsigned char operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		void resize(const osg::Vec3i& lbound_, const osg::Vec3i& size_, bool customShape_, bool preserved);

	public:
		std::string id; //!< the polyhedron id
		int shape; //!< the polyhedron shape. \sa PolyhedronShape

		std::string objType; //!< the object type
		int flags; //!< the polyhedron flags. \sa PolyhedronFlags
		int movement; //!< the polyhedron movement. \sa PolyhedronMovement
		int controller; //!< the polyhedron controller. \sa PolyhedronController
		MapPosition pos; //!< the start position

		osg::Vec3i lbound; //!< lower bound of polyhedron data, which in turn determines the center of polyhedron
		osg::Vec3i size; //!< the size of polyhedron

		bool customShapeEnabled; //!< use custom shape. if it is true then customShape is used, otherwise the polyhedron is a solid cuboid.
		std::vector<unsigned char> customShape; //!< the custom shape

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, shape);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, flags);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, movement);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, controller);
		UTIL_ADD_BYREF_GETTER_SETTER(MapPosition, pos);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, lbound);
		UTIL_ADD_BYREF_GETTER_SETTER(osg::Vec3i, size);
		UTIL_ADD_BYVAL_GETTER_SETTER(bool, customShapeEnabled);
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<unsigned char>, customShape);

	public:
		//the following properties don't save to file and is generated at runtime
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
	};

}
