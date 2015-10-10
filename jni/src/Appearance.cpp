#include "Appearance.h"
#include "SimpleGeometry.h"
#include "XMLReaderWriter.h"
#include "util_err.h"
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
		, diffuse(0, 0, 0, 1)
		, specular(0, 0, 0, 1)
		, emissive(0, 0, 0, 1)
		, specularHardness(0)
		, meshType(0)
		, pos()
		, rot()
		, scale(1, 1, 1)
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

	//TODO: copy constructor
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
		case TRANSFORM:
		{
			if (subNodes.empty()) break;
			
			osg::ref_ptr<osg::Group> gp;
			if (type == TRANSFORM) {
				osg::Matrix mat;
				mat.makeScale(scale);
				mat.postMultRotate(osg::Quat(rot.x(), osg::X_AXIS, rot.y(), osg::Y_AXIS, rot.z(), osg::Z_AXIS));
				mat.postMultTranslate(pos);

				gp = new osg::MatrixTransform(mat);
			} else {
				gp = new osg::Group;
			}
			for (size_t i = 0; i < subNodes.size(); i++) {
				gp->addChild(subNodes[i]->getOrCreateInstance(shape));
			}
			node = gp.get();

			if (type == SHADER) {
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
		case MESH:
		{
			osg::ref_ptr<osg::LOD> lodNode;
			osg::ref_ptr<osg::Geode> geode;

			osg::Vec3 p1 = pos - osg::Vec3(scale.x()*center.x(), scale.y()*center.y(), scale.z()*center.z());
			osg::Vec3 p2 = p1 + scale;

			if (meshType == MESH_CUBE) {
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
						//FIXME: wireframe should be affected by material color?
						//g->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); //???
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
			}

			if (lodNode.valid()) node = lodNode.get();
			else if (geode.valid()) node = geode.get();
			else break;

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

	bool Appearance::load(const XMLNode* node){
		//check node type
		for (;;) {
			if (node->name == "appearance") type = APPEARANCE;
			else if (node->name == "shader") type = SHADER;
			else if (node->name == "transform") type = TRANSFORM;
			else if (node->name == "mesh") type = MESH;
			else {
				UTIL_WARN "unrecognized node name: " << node->name << std::endl;
				return false;
			}

			//if it is appearance node and only have one child then load recursively
			if (type == APPEARANCE && node->subNodes.size() == 1) {
				node = node->subNodes[0].get();
				continue;
			}

			break;
		}

		if (type == MESH) {
			std::string s = node->getAttr("type", std::string("cube"));
			if (s == "cube") meshType = MESH_CUBE;
			else {
				UTIL_WARN "unrecognized mesh type: " << s << std::endl;
				return false;
			}

			pos = node->getAttrOsgVec("p", osg::Vec3());
			rot = node->getAttrOsgVec("r", osg::Vec3());
			scale = node->getAttrOsgVec("s", osg::Vec3(1, 1, 1));
			center = node->getAttrOsgVec("c", osg::Vec3());
			bevel = node->getAttr("bevel", 0.0f);

			if (node->getAttr("solidAndWireframe", false)) {
				solid = true;
				wireframe = true;
				solidColor = node->getAttrOsgVec("color", osg::Vec3(1, 1, 1));
				wireframeColor = node->getAttrOsgVec("wireframeColor", osg::Vec3(1, 1, 1));
			} else if (node->getAttr("wireframe", false)) {
				solid = false;
				wireframe = true;
				if (node->attributes.find("wireframeColor") != node->attributes.end()) {
					wireframeColor = node->getAttrOsgVec("wireframeColor", osg::Vec3(1, 1, 1));
				} else {
					wireframeColor = node->getAttrOsgVec("color", osg::Vec3(1, 1, 1));
				}
			} else {
				solid = node->getAttr("solid", true);
				wireframe = false;
				solidColor = node->getAttrOsgVec("color", osg::Vec3(1, 1, 1));
			}

			lod = node->getAttr("lod", false);
		} else {
			switch (type) {
			case SHADER:
				ambient = node->getAttrOsgVec("ambient", osg::Vec4(0, 0, 0, 1));
				diffuse = node->getAttrOsgVec("diffuse", osg::Vec4(0, 0, 0, 1));
				specular = node->getAttrOsgVec("specular", osg::Vec4(0, 0, 0, 1));
				emissive = node->getAttrOsgVec("emissive", osg::Vec4(0, 0, 0, 1));
				specularHardness = node->getAttr("specularHardness", 0.0f);
				break;
			case TRANSFORM:
				pos = node->getAttrOsgVec("p", osg::Vec3());
				rot = node->getAttrOsgVec("r", osg::Vec3());
				scale = node->getAttrOsgVec("s", osg::Vec3(1, 1, 1));
				break;
			}

			//load subnodes
			for (size_t i = 0; i < node->subNodes.size(); i++) {
				osg::ref_ptr<Appearance> a = new Appearance;
				if (a->load(node->subNodes[i].get())) {
					subNodes.push_back(a);
				}
			}
		}

		return true;
	}

	REG_OBJ_WRAPPER(gfx, Appearance, "")
	{
		ADD_INT_SERIALIZER(type, 0);
		ADD_VEC4_SERIALIZER(ambient, osg::Vec4(0, 0, 0, 1));
		ADD_VEC4_SERIALIZER(diffuse, osg::Vec4(0, 0, 0, 1));
		ADD_VEC4_SERIALIZER(specular, osg::Vec4(0, 0, 0, 1));
		ADD_VEC4_SERIALIZER(emissive, osg::Vec4(0, 0, 0, 1));
		ADD_FLOAT_SERIALIZER(specularHardness, 0);
		ADD_INT_SERIALIZER(meshType, 0);
		ADD_VEC3_SERIALIZER(pos, osg::Vec3());
		ADD_VEC3_SERIALIZER(rot, osg::Vec3());
		ADD_VEC3_SERIALIZER(scale, osg::Vec3(1.0f, 1.0f, 1.0f));
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
