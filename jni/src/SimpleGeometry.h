#pragma once

#include "util_object.h"
#include <osg/Geometry>

namespace gfx {

	osg::Geometry* createCube(const osg::Vec3& p1, const osg::Vec3& p2, bool wireframe, float bevel, const osg::Vec3& color);

	/// currently unused

	class SimpleGeometry : public osg::Object {
	protected:
		virtual ~SimpleGeometry();
	public:
		META_Object(gfx, SimpleGeometry);

		SimpleGeometry();
		SimpleGeometry(const SimpleGeometry& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
	};

}
