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
#include <stdio.h>
#include <string.h>

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
		, scale2(1, 1, 1)
		, angles()
		, center()
		, bevel(0)
		, segments(1)
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

	bool Appearance::hasLOD() const {
		if (type != MESH) return false;

		switch (meshType) {
		case CUBE:
		case RECTANGLE:
			if (lod && bevel >= 1E-6f) return true;
		}

		for (size_t i = 0, m = subNodes.size(); i < m; i++) {
			if (subNodes[i].valid() && subNodes[i]->hasLOD()) return true;
		}

		return false;
	}

	SimpleGeometry* Appearance::createSimpleGeometry(SimpleGeometry* existing, int shape, bool isLODed) const {
		osg::ref_ptr<SimpleGeometry> g = existing;

		switch (meshType) {
		case POLYHEDRON:
			if (!g.valid()) g = new SimpleGeometry;
			{
				const int vertexCount = _vertices.size() / 3;
				std::vector<int> v1, v2;
				for (int i = 0, m = _faces.size(); i < m; i++) {
					const int n = _faces[i];
					if (n <= 2 || i + n >= m) break;
					bool err = false;
					for (int j = 0; j < n; j++) {
						int idx = _faces[++i];
						if (idx < 0 || idx >= vertexCount) {
							err = true;
							break;
						}
						v1.push_back(idx);
					}
					if (err) break;
					v2.push_back(n);
				}
				if (!v2.empty()) {
					g->addPolyhedron((osg::Vec3*)(&(_vertices[0])), vertexCount, &(v1[0]), &(v2[0]), v2.size());
				}
			}
			break;
		case CUBE:
			if (!g.valid()) g = new SimpleGeometry;
			{
				osg::Vec3 p1 = pos - osg::Vec3(scale.x()*center.x(), scale.y()*center.y(), scale.z()*center.z());
				osg::Vec3 p2 = p1 + scale;
				g->addCube(p1, p2, isLODed ? 0.0f : bevel);
			}
			break;
		case POLYGON:
			if (!g.valid()) g = new SimpleGeometry;
			{
				const int vertexCount = _vertices.size() / 3;
				if (!_triangulation.valid() || _triangulation->valid(vertexCount)) {
					g->addPolygon((osg::Vec3*)(&(_vertices[0])), vertexCount, _triangulation.get());
				}
			}
			break;
		case RECTANGLE:
			if (!g.valid()) g = new SimpleGeometry;
			{
				osg::Vec3 p1 = pos - osg::Vec3(scale.x()*center.x(), scale.y()*center.y(), scale.z()*center.z());
				osg::Vec3 p2 = p1 + scale;
				g->addRect(p1, p2, isLODed ? 0.0f : bevel, segments);
			}
			break;
		case ELLIPSE:
			if (!g.valid()) g = new SimpleGeometry;
			{
				osg::Vec3 p1 = pos - osg::Vec3(scale.x()*(center.x() - 0.5f), scale.y()*(center.y() - 0.5f), scale.z()*center.z());
				g->addEllipse(p1, osg::Vec2(scale.x()*0.5f, scale.y()*0.5f), segments < 3 ? 3 : segments);
			}
			break;
		case CHORD:
			if (!g.valid()) g = new SimpleGeometry;
			{
				osg::Vec3 p1 = pos - osg::Vec3(scale.x()*(center.x() - 0.5f), scale.y()*(center.y() - 0.5f), scale.z()*center.z());
				g->addChord(p1, osg::Vec2(scale.x()*0.5f, scale.y()*0.5f), angles.x(), angles.y(), segments < 1 ? 1 : segments);
			}
			break;
		case PIE:
			if (!g.valid()) g = new SimpleGeometry;
			{
				osg::Vec3 p1 = pos - osg::Vec3(scale.x()*(center.x() - 0.5f), scale.y()*(center.y() - 0.5f), scale.z()*center.z());
				g->addPie(p1, osg::Vec2(scale.x()*0.5f, scale.y()*0.5f), angles.x(), angles.y(), segments < 1 ? 1 : segments, osg::Vec2(scale2.x()*0.5f, scale2.y()*0.5f));
			}
			break;
		case PRISM:
		{
			for (size_t i = 0, m = subNodes.size(); i < m; i++) {
				if (subNodes[i].valid()) {
					osg::ref_ptr<SimpleGeometry> g2 = subNodes[i]->createSimpleGeometry(g.get(), shape, isLODed);
					g = g2;
				}
			}
			if (g.valid()) {
				osg::ref_ptr<SimpleGeometry> g2 = new SimpleGeometry;
				//FIXME: ad-hoc
				g2->addPrism(g.get(), false, true, osg::Vec3(0, 0, scale.z()),
					scale.x() < 0 ? scale.y() : scale.x(), scale.x() < 0);
				g = g2;
			}
		}
			break;
		case PYRAMID:
		{
			for (size_t i = 0, m = subNodes.size(); i < m; i++) {
				if (subNodes[i].valid()) {
					osg::ref_ptr<SimpleGeometry> g2 = subNodes[i]->createSimpleGeometry(g.get(), shape, isLODed);
					g = g2;
				}
			}
			if (g.valid()) {
				osg::ref_ptr<SimpleGeometry> g2 = new SimpleGeometry;
				//FIXME: ad-hoc
				g2->addPyramid(g.get(), false, true, osg::Vec3(0, 0, scale.z()),
					osg::Vec3());
				g = g2;
			}
		}
			break;
		}

		return g.release();
	}

	osg::Geode* Appearance::createGeodeFromSimpleGeometry(SimpleGeometry* sg) const {
		if (sg == NULL) return NULL;

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;

		if (solid) {
			osg::Geometry* g = sg->createFaces(solidColor, false, true);
			if (g) {
				if (wireframe) {
					g->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f));
				}
				geode->addDrawable(g);
			}
		}

		if (wireframe) {
			osg::Geometry* g = sg->createEdges(wireframeColor);
			if (g) {
				//FIXME: wireframe should be affected by material color?
				//g->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); //???
				geode->addDrawable(g);
			}
		}

		return geode.release();
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
			osg::ref_ptr<osg::Geode> geode;

			if (hasLOD()) {
				osg::ref_ptr<osg::LOD> lodNode = new osg::LOD;
				osg::ref_ptr<SimpleGeometry> sg = createSimpleGeometry(NULL, shape, false);
				lodNode->addChild(createGeodeFromSimpleGeometry(sg.get()), 0.0f, 50.0f);
				sg = createSimpleGeometry(NULL, shape, true);
				lodNode->addChild(createGeodeFromSimpleGeometry(sg.get()), 50.0f, FLT_MAX);
				node = lodNode;
			} else {
				osg::ref_ptr<SimpleGeometry> sg = createSimpleGeometry(NULL, shape, false);
				node = createGeodeFromSimpleGeometry(sg.get());
			}

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
		case MESH:
		{
			std::string s = node->getAttr("type", std::string("cube"));
			if (s.empty() || s == "polyhedron") meshType = POLYHEDRON;
			else if (s == "cube") meshType = CUBE;
			else if (s == "polygon") meshType = POLYGON;
			else if (s == "rectangle") meshType = RECTANGLE;
			else if (s == "ellipse") meshType = ELLIPSE;
			else if (s == "chord") meshType = CHORD;
			else if (s == "pie") meshType = PIE;
			else if (s == "prism") meshType = PRISM;
			else if (s == "pyramid") meshType = PYRAMID;
			else {
				UTIL_WARN "unrecognized mesh type: " << s << std::endl;
				return false;
			}

			pos = node->getAttrOsgVec("p", osg::Vec3());
			rot = node->getAttrOsgVec("r", osg::Vec3());
			scale = node->getAttrOsgVec("s", osg::Vec3(1, 1, 1));
			scale2 = node->getAttrOsgVec("s2", osg::Vec3(1, 1, 1));
			angles = node->getAttrOsgVec("a", osg::Vec2(1, 1));
			center = node->getAttrOsgVec("c", osg::Vec3());
			bevel = node->getAttr("bevel", 0.0f);
			segments = node->getAttr("segments", 1);

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
		}
			break;
		}

		if (type == MESH && meshType == POLYHEDRON) {
			for (size_t i = 0; i < node->subNodes.size(); i++) {
				XMLNode *subnode = node->subNodes[i];
				if (subnode->name == "vertices") {
					loadVertices(subnode);
				} else if (subnode->name == "faces") {
					loadFaces(subnode);
				} else {
					UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
				}
			}
		} else if (type == MESH && meshType == POLYGON) {
			for (size_t i = 0; i < node->subNodes.size(); i++) {
				XMLNode *subnode = node->subNodes[i];
				if (subnode->name == "vertices") {
					loadVertices(subnode);
				} else if (subnode->name == "triangulation") {
					osg::ref_ptr<Triangulation> t = new Triangulation;
					if (t->load(subnode)) {
						_triangulation = t;
					}
				} else {
					UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
				}
			}
		} else {
			//load subnodes, although sometimes these nodes are ignored
			for (size_t i = 0; i < node->subNodes.size(); i++) {
				osg::ref_ptr<Appearance> a = new Appearance;
				if (a->load(node->subNodes[i].get())) {
					subNodes.push_back(a);
				}
			}
		}

		return true;
	}

	void Appearance::loadVertices(const XMLNode* node) {
		if (!node->contents.empty()) {
			const char* s = node->contents.c_str();
			for (;;) {
				float f = 0.0f;
				if (sscanf(s, "%f", &f) != 1) break;
				_vertices.push_back(f);
				s = strchr(s, ',');
				if (s == NULL) break;
				s++;
				if (*s == 0) break;
			}
		}
	}

	void Appearance::loadFaces(const XMLNode* node) {
		if (!node->contents.empty()) {
			const char* s = node->contents.c_str();
			for (;;) {
				int i = 0;
				if (sscanf(s, "%d", &i) != 1) break;
				_faces.push_back(i);
				s = strchr(s, ',');
				if (s == NULL) break;
				s++;
				if (*s == 0) break;
			}
		}
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
		ADD_VEC3_SERIALIZER(scale2, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VEC2_SERIALIZER(angles, osg::Vec2());
		ADD_VEC3_SERIALIZER(center, osg::Vec3());
		ADD_FLOAT_SERIALIZER(bevel, 0);
		ADD_INT_SERIALIZER(segments, 1);
		ADD_BOOL_SERIALIZER(solid, true);
		ADD_BOOL_SERIALIZER(wireframe, false);
		ADD_BOOL_SERIALIZER(lod, false);
		ADD_VEC3_SERIALIZER(solidColor, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VEC3_SERIALIZER(wireframeColor, osg::Vec3(1.0f, 1.0f, 1.0f));
		ADD_VECTOR_SERIALIZER(subNodes, std::vector<osg::ref_ptr<Appearance> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}

}
