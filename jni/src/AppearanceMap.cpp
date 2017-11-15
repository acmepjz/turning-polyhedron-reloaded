#include "AppearanceMap.h"
#include "util_err.h"
#include "XMLReaderWriter.h"
#include <osgDB/ObjectWrapper>

namespace gfx {

	AppearanceMap::AppearanceMap() {

	}

	AppearanceMap::AppearanceMap(const AppearanceMap& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, parent(other.parent) // FIXME: always shallow copy?
	{
		util::copyMap(map, other.map, copyop);
	}

	AppearanceMap::~AppearanceMap() {

	}

	bool AppearanceMap::add(Appearance* obj){
		if (!obj) {
			UTIL_ERR "object is NULL" << std::endl;
			return false;
		}

		if (lookup(obj->id, true)) {
			UTIL_WARN "object id '" << obj->id << "' already defined, will be redefined to a new object" << std::endl;
		}

		map[obj->id] = obj;
		return true;
	}

	Appearance* AppearanceMap::lookup(const std::string& name, bool noWarning){
		Map::iterator it = map.find(name);
		if (it == map.end()) {
			if (parent.valid()) {
				return parent->lookup(name, noWarning);
			} else {
				if (!noWarning) UTIL_WARN "id '" << name << "' not found" << std::endl;
				return NULL;
			}
		}
		return it->second.get();
	}

	bool AppearanceMap::load(const XMLNode* node, AppearanceMap* _template, const std::string& _defaultId, const osg::Vec3& _defaultSize) {
		bool ret = true;

		for (size_t i = 0; i < node->subNodes.size(); i++) {
			if (!loadAppearance(node->subNodes[i].get(), _template, _defaultId, _defaultSize)) ret = false;
		}

		return ret;
	}

	bool AppearanceMap::loadAppearance(const XMLNode* node, AppearanceMap* _template, const std::string& _defaultId, const osg::Vec3& _defaultSize) {
		osg::ref_ptr<Appearance> ot = new Appearance;
		if (ot->load(node, _template, _defaultId, _defaultSize)) {
			add(ot.get());
			return true;
		} else {
			UTIL_WARN "failed to load appearance" << std::endl;
			return false;
		}
	}

	REG_OBJ_WRAPPER(gfx, AppearanceMap, "")
	{
		ADD_OBJECT_SERIALIZER(parent, AppearanceMap, 0);
		ADD_MAP_SERIALIZER(map, AppearanceMap::Map, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
