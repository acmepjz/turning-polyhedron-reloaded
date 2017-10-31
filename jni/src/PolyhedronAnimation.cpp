#include "PolyhedronAnimation.h"
#include "Polyhedron.h"
#include "MapPosition.h"
#include "util_err.h"
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgFX/Outline>
#include <osgDB/ObjectWrapper>

namespace game {

	PolyhedronAnimation::PolyhedronAnimation(Polyhedron* poly, MoveDirection dir, AnimationType type)
		: _poly(poly)
		, _t(0)
		, _maxt(8)
		, _type(type)
	{
		switch (type) {
		case ROLLING:
			poly->pos.getTransformAnimation(poly, MoveDirection(dir ^ 1), _mat1, _quat, _mat2);
			break;
		case MOVING:
		{
			PolyhedronPosition oldPos = poly->pos;
			oldPos.applyTransform(poly, _mat2);
			switch (dir) {
			case MOVE_NEG_X: oldPos.pos.x()++; break;
			case MOVE_POS_X: oldPos.pos.x()--; break;
			case MOVE_NEG_Y: oldPos.pos.y()++; break;
			case MOVE_POS_Y: oldPos.pos.y()--; break;
			case MOVE_NEG_Z: oldPos.pos.z()++; break;
			case MOVE_POS_Z: oldPos.pos.z()--; break;
			}
			oldPos.applyTransform(poly, _mat1);
			for (int i = 0; i < 16; i++) {
				_mat2.ptr()[i] -= _mat1.ptr()[i];
			}
		}
			break;
		}
	}

	PolyhedronAnimation::~PolyhedronAnimation() {
	}

	bool PolyhedronAnimation::update() {
		if (_t < _maxt) {
			_t++;

			switch (_type) {
			case ROLLING:
				if (_t >= _maxt) {
					_poly->updateTransform();
				} else {
					osg::Matrix mat = _mat1;
					osg::Quat q;
					q.slerp(float(_t) / float(_maxt), _quat, osg::Quat());
					mat.postMultRotate(q);
					mat.postMult(_mat2);
					_poly->_trans->setMatrix(mat);
				}
				break;
			case MOVING:
				if (_t >= _maxt) {
					_poly->updateTransform();
				} else {
					float t = float(_t) / float(_maxt);
					osg::Matrix mat = _mat1;
					for (int i = 0; i < 16; i++) {
						mat.ptr()[i] += t*_mat2.ptr()[i];
					}
					_poly->_trans->setMatrix(mat);
				}
				break;
			case FLASHING:
			{
				osgFX::Outline *outline = dynamic_cast<osgFX::Outline*>(_poly->_appearance.get());
				if (outline) {
					if (_t >= _maxt) {
						outline->setEnabled(false);
						outline->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
					} else {
						outline->setEnabled(((_t - 1) & 2) ? false : true);
						outline->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}
			}
				break;
			default:
				UTIL_WARN "unrecognized animation type: " << int(_type) << std::endl;
				return false;
			}

			return true;
		}

		return false;
	}

}
