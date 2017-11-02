#pragma once

#include <osg/Referenced>

namespace game {
	class Polyhedron;
}

class MousePickingData : public osg::Referenced {
protected:
	~MousePickingData();
public:
	MousePickingData();

	game::Polyhedron *polyhedron;
	int polyhedronIndex;
};
