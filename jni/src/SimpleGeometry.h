#pragma once

#include <osg/Geometry>

namespace geom {

	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color);

}
