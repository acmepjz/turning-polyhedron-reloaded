#pragma once

#include "Appearance.h"
#include <string>
#include <map>

namespace gfx {

	/// A map used to look up appearances

	class AppearanceMap : public osg::Object {
	protected:
		virtual ~AppearanceMap();
	public:
		META_Object(game, AppearanceMap);

		AppearanceMap();
		AppearanceMap(const AppearanceMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		//! Add an appearance to map.
		bool add(Appearance* obj);

		//! Find an object with given name.
		Appearance* lookup(const std::string& name, bool noWarning = false);

		//! load from XML node, assume the node is called `appearances`
		bool load(const XMLNode* node, AppearanceMap* _template, const std::string& _defaultId, const osg::Vec3& _defaultSize);

		//! load from XML node, will check if the node is accepted by \ref gfx::Appearance::load()
		bool loadAppearance(const XMLNode* node, AppearanceMap* _template, const std::string& _defaultId, const osg::Vec3& _defaultSize);

	public:
		osg::ref_ptr<AppearanceMap> parent;

		typedef std::map<std::string, osg::ref_ptr<Appearance> > Map;
		Map map;

		UTIL_ADD_OBJ_GETTER_SETTER(AppearanceMap, parent);
		UTIL_ADD_BYREF_GETTER_SETTER(Map, map);
	};

}
