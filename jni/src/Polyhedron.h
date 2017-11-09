#pragma once

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Vec3i>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "Appearance.h"
#include "PolyhedronPosition.h"
#include "EventHandler.h"
#include "util_object.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

class XMLNode;

namespace game {

	class Level;
	class ObjectType;
	class TileType;
	class PolyhedronAnimation;

	enum MoveDirection;

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

		struct PolyhedronFlagsAndNames {
			const char* name;
			bool defaultValue;
			int flags;
		};

		static const PolyhedronFlagsAndNames polyhedronFlagsAndNames[]; //!< a non-exhaustive list of available polyhedron flags and their names
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

		struct HitTestResult {
			struct Position {
				MapData *_map;
				osg::Vec3i position;

				bool operator<(const Position& other) const {
					if (_map < other._map) return true;
					if (_map > other._map) return false;

					if (position[0] < other.position[0]) return true;
					if (position[0] > other.position[0]) return false;

					if (position[1] < other.position[1]) return true;
					if (position[1] > other.position[1]) return false;

					if (position[2] < other.position[2]) return true;
					return false;
				}

				bool operator==(const Position& other) const {
					return _map == other._map && position == other.position;
				}
			};

			std::map<Position, osg::Object*> supporterPosition; //!< the tiles which support this polyhedron, can be \ref TileType or \ref Polyhedron (which can be duplicated).
			std::set<Polyhedron*> supporterPolyhedron; //!< the polyhedra which support this polyhedron.
			std::map<Position, TileType*> hitTestPosition; //!< the hit-tested tiles.
		};

		enum HitTestReason {
			HITTEST_VALID,
			HITTEST_FALL,
			HITTEST_BLOCKED,
		};

		/** check if it is stable and not blocked at given position. (experimental, only works for cuboid polyhedron)
		\param parent The parent
		\param pos The position
		\param[out] hitTestResult (optional) the hit test result. NOTE: this is valid only when the return value is true.
		\param[out] reason (optional) the reason why it is invalid
		*/
		bool valid(const Level* parent, const PolyhedronPosition& pos, HitTestResult* hitTestResult = 0, HitTestReason* reason = 0) const;

		/** check if it is stable and not blocked at current position. (experimental, only works for cuboid polyhedron)
		\param parent The parent
		\param[out] hitTestResult (optional) the hit test result. NOTE: this is valid only when the return value is true.
		\param[out] reason (optional) the reason why it is invalid
		*/
		bool valid(const Level* parent, HitTestResult* hitTestResult = 0, HitTestReason* reason = 0) const {
			return valid(parent, pos, hitTestResult, reason);
		}

		void init(Level* parent);

		bool load(const XMLNode* node, Level* parent, MapData* mapData, gfx::AppearanceMap* _template); //!< load from XML node, assume the node is called `polyhedron`

		void setSelected(bool selected);

		int weight() const; //!< get the weight of polyhedron. NOTE: anything non EMPTY gets a weight 1.

		/** update animation
		\return if it is animating
		*/
		bool update(Level* parent);

		/** call when the tile changed
		\return if it is animating
		*/
		bool onTileDirty(Level* parent);

		/** call when the polyhedron is removed
		\param parent the level
		\param type the type, e.g. "breakdown", "fall"
		\return if it is animating
		*/
		bool onRemove(Level* parent, const std::string& type);

		void updateVisible(); //!< call this when the VISIBLE flags changed

		void processEvent(Level* parent, EventDescription* evt) {
			for (size_t i = 0; i < events.size(); i++) {
				events[i]->processEvent(parent, evt);
			}
		}

	private:
		//void raiseOnEnterOrOnLeave(int wt, HitTestResult& hitTestResult);

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

		std::vector<osg::ref_ptr<EventHandler> > events; //!< the events

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
		UTIL_ADD_BYREF_GETTER_SETTER(std::vector<osg::ref_ptr<EventHandler> >, events);

	public:
		//the following properties don't save to file and is generated at runtime
		osg::ref_ptr<osg::Node> _appearance; //!< the appearance
		osg::ref_ptr<osg::MatrixTransform> _trans; //!< the appearance with correct transform
		ObjectType* _objType;

		std::vector<osg::ref_ptr<PolyhedronAnimation> > _animations;
		int _currentAnimation;
	};

}
