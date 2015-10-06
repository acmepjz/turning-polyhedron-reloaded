#include "Appearance.h"
#include "SimpleGeometry.h"
#include <osg/Group>
#include <osg/Geode>
#include <osg/PolygonOffset>
#include <osg/Material>
#include <osg/LOD>
#include <osg/MatrixTransform>

namespace gfx {

	Appearance::Appearance()
		: type(0)
		, ambient(0.0f, 0.0f, 0.0f, 1.0f)
		, diffuse(1.0f, 1.0f, 1.0f, 1.0f)
		, specular(0.0f, 0.0f, 0.0f, 1.0f)
		, emissive(0.0f, 0.0f, 0.0f, 1.0f)
		, specularHardness(0.0f)
		, pos()
		, rot()
		, scale(1.0f, 1.0f, 1.0f)
		, center()
		, bevel(0.0f)
		, solid(true)
		, wireframe(false)
		, lod(false)
		, solidColor()
		, wireframeColor()
	{
	}

	Appearance::~Appearance()
	{
	}

	Appearance::Appearance(const Appearance& other, const osg::CopyOp& copyop)
		: osg::Object(other, copyop)
	{
	}

	osg::Node* Appearance::getOrCreateInstance(int shape)
	{
		//check if the instance is already created
		{
			std::map<int, osg::ref_ptr<osg::Node> >::iterator it = _instances.find(shape);
			if (it != _instances.end()) {
				return it->second.get();
			}
		}

		osg::ref_ptr<osg::Node> node;

		switch (type) {
		case APPEARANCE:
		case SHADER:
		{
			if (subNodes.empty()) break;
			
			osg::ref_ptr<osg::Group> gp = new osg::Group;
			for (size_t i = 0; i < subNodes.size(); i++) {
				gp->addChild(subNodes[i]->getOrCreateInstance(shape));
			}
			node = gp.get();

			if (type == SHADER && node.valid()) {
				if (!_stateSet.valid()) {
					//create new state set
					_stateSet = new osg::StateSet;

					//create material
					osg::ref_ptr<osg::Material> mat = new osg::Material;
					mat->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
					mat->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
					mat->setSpecular(osg::Material::FRONT_AND_BACK, specular);
					mat->setEmission(osg::Material::FRONT_AND_BACK, emissive);
					mat->setShininess(osg::Material::FRONT_AND_BACK, specularHardness);

					_stateSet->setAttributeAndModes(mat.get());
				}
				node->setStateSet(_stateSet.get());
			}
			break;
		}
		case MESH_CUBE:
		{
			osg::ref_ptr<osg::LOD> lodNode;
			osg::ref_ptr<osg::Geode> geode;

			osg::Vec3 p1 = pos - osg::Vec3(scale.x()*center.x(), scale.y()*center.y(), scale.z()*center.z());
			osg::Vec3 p2 = p1 + scale;

			for (int i = 0; i < 2; i++) {
				// check if we need to create bevel geometry
				if (i == 0 && bevel <= 1E-6f) continue;

				// create geometry
				if (geode.valid()) break;
				geode = new osg::Geode;
				if (solid) {
					osg::Geometry* g = gfx::createCube(p1, p2, false, i ? 0.0f : bevel, solidColor);
					if (wireframe) {
						g->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
					}
					geode->addDrawable(g);
				}
				if (wireframe) {
					osg::Geometry* g = gfx::createCube(p1, p2, true, i ? 0.0f : bevel, wireframeColor);
					g->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
					geode->addDrawable(g);
				}

				// check if we need to create LOD
				if (lod) {
					if (i == 0) {
						lodNode = new osg::LOD;
						lodNode->addChild(geode.release(), 0.0f, 50.0f);
					} else {
						lodNode->addChild(geode.release(), 50.0f, FLT_MAX);
					}
				}
			}

			if (lodNode.valid()) node = lodNode.get();
			else node = geode.get();

			if (rot.x() < -1E-6f || rot.x() > 1E-6f
				|| rot.x() < -1E-6f || rot.x() > 1E-6f
				|| rot.x() < -1E-6f || rot.x() > 1E-6f)
			{
				osg::Matrix mat;
				mat.makeRotate(rot.x(), osg::X_AXIS, rot.y(), osg::Y_AXIS, rot.z(), osg::Z_AXIS);

				osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform(mat);
				mt->addChild(node.get());
				node = mt.get();
			}

			break;
		}
		}

		//save the new instance
		_instances[shape] = node;

		return node.release();
	}

}
