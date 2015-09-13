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

		std::string objType; //!< the object type
		int flags; //!< the polyhedron flags
		MapPosition pos; //!< the start position

		osg::Vec3i lbound; //!< lower bound of polyhedron data, which in turn determines the center of polyhedron
		osg::Vec3i size; //!< the size of polyhedron

		bool customShapeEnabled; //!< use custom shape. if it is true then \c customShape is used, otherwise the polyhedron is a solid cuboid.
		std::vector<unsigned char> customShape; //!< the custom shape

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, id);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, objType);
		UTIL_ADD_BYVAL_GETTER_SETTER(int, flags);
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
