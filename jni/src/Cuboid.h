#pragma once

#include "Polyhedron.h"
#include <osg/Vec3i>
#include <vector>

namespace game {

	class Cuboid :
		public Polyhedron
	{
	protected:
		virtual ~Cuboid();
	public:
		META_Object(game, Cuboid);

		Cuboid();
		Cuboid(const Cuboid& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		char& operator()(int x, int y, int z);
		char& operator()(const osg::Vec3i& p) {
			return operator()(p.x(), p.y(), p.z());
		}
		char operator()(int x, int y, int z) const;
		char operator()(const osg::Vec3i& p) const {
			return operator()(p.x(), p.y(), p.z());
		}

		void resize(const osg::Vec3i& size_, bool customShape_, bool preserved);

	public:
		osg::Vec3i size; //!< the size of polyhedron

		bool customShapeEnabled;
		std::vector<char> customShape;
	};

}
