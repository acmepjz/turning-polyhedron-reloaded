#pragma once

#include <osg/Matrix>
#include <osg/Quat>
#include <vector>
#include <string>
#include "EventDescription.h"

namespace osgDB {
	class InputStream;
	class OutputStream;
}

class XMLNode;

namespace game {

	class Level;
	class ObjectType;
	class Polyhedron;

	enum MoveDirection;

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

		std::vector<osg::ref_ptr<EventDescription> > _eventWhenAninationFinished;
	};

}
