#include "Cuboid.h"
#include <osgDB/ObjectWrapper>

namespace game {

	Cuboid::Cuboid()
		: size(1, 1, 2)
		, customShapeEnabled(false)
		, customShape(2, 1)
	{
	}

	Cuboid::Cuboid(const Cuboid& other, const osg::CopyOp& copyop)
		: Polyhedron(other, copyop)
		, size(other.size)
		, customShapeEnabled(other.customShapeEnabled)
		, customShape(other.customShape)
	{

	}


	Cuboid::~Cuboid()
	{
	}

	char& Cuboid::operator()(int x, int y, int z){
		int idx = (z*size.y() + y)*size.x() + x;
		return customShape[idx];
	}

	char Cuboid::operator()(int x, int y, int z) const{
		int idx = (z*size.y() + y)*size.x() + x;
		return customShape[idx];
	}

	void Cuboid::resize(const osg::Vec3i& size_, bool customShape_, bool preserved){
		customShapeEnabled = customShape_;

		if (!preserved || !customShape_) {
			size = size_;
			customShape.resize(size_.x()*size_.y()*size_.z(), 1);
			return;
		}

		std::vector<char> tmp = customShape;
		customShape.resize(size_.x()*size_.y()*size_.z(), 0);

#define EX(X) e##X = (size.X() < size_.X()) ? \
	(size.X()) : (size_.X())
		const int EX(x), EX(y), EX(z);
#undef EX

		for (int z = 0; z < ez; z++) {
			for (int y = 0; y < ey; y++) {
				for (int x = 0; x < ex; x++) {
					int old_idx = (z*size.y() + y)*size.x() + x;
					int new_idx = (z*size_.y() + y)*size_.x() + x;
					customShape[new_idx] = tmp[old_idx];
				}
			}
		}

		size = size_;
	}

	REG_OBJ_WRAPPER(game, Cuboid, "game::Polyhedron ")
	{
		ADD_VEC3I_SERIALIZER(size, osg::Vec3i(1, 1, 2));
		ADD_BOOL_SERIALIZER(customShapeEnabled, false);
		ADD_VECTOR_SERIALIZER(customShape, std::vector<char>, osgDB::BaseSerializer::RW_UCHAR, 32);
	}

}
