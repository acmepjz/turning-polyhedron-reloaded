#include "Appearance.h"
#include "SimpleGeometry.h"
#include <osg/Group>
#include <osg/Geode>
#include <osg/PolygonOffset>
#include <osg/Material>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osgDB/ObjectWrapper>

namespace gfx {

	Appearance::Appearance()
		: type(0)
		, ambient(0, 0, 0, 1)
		, diffuse(1, 1, 1, 1)
		, specular(0, 0, 0, 1)
		, emissive(0, 0, 0, 1)
		, specularHardness(0)
		, pos()
		, rot()
		, size(1, 1, 1)
		, center()
		, bevel(0)
		, solid(true)
		, wireframe(false)
		, lod(false)
		, solidColor(1, 1, 1)
		, wireframeColor(1, 1, 1)
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

			osg::Vec3 p1 = pos - osg::Vec3(size.x()*center.x(), size.y()*center.y(), size.z()*center.z());
			osg::Vec3 p2 = p1 + size;

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

	REG_OBJ_WRAPPER(gfx, Appearance, "")
	{
		ADD_INT_SERIALIZER(type, 0);
		ADD_VEC4_SERIALIZER(ambient, osg::Vec4(0, 0, 0, 1));
		ADD_VEC4_SERIALIZER(diffuse, osg::Vec4(1, 1, 1, 1));
		ADD_VEC4_SERIALIZER(specular, osg::Vec4(0, 0, 0, 1));
		ADD_VEC4_SERIALIZER(emissive, osg::Vec4(0, 0, 0, 1));
		ADD_FLOAT_SERIALIZER(specularHardness, 0);
		ADD_VEC3_SERIALIZER(pos, osg::Vec3());
		ADD_VEC3_SERIALIZER(rot, osg::Vec3());
		ADD_VEC3_SERIALIZER(size, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VEC3_SERIALIZER(center, osg::Vec3());
		ADD_FLOAT_SERIALIZER(bevel, 0);
		ADD_BOOL_SERIALIZER(solid, true);
		ADD_BOOL_SERIALIZER(wireframe, false);
		ADD_BOOL_SERIALIZER(lod, false);
		ADD_VEC3_SERIALIZER(solidColor, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VEC3_SERIALIZER(wireframeColor, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VECTOR_SERIALIZER(subNodes, std::vector<osg::ref_ptr<Appearance> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}

}
