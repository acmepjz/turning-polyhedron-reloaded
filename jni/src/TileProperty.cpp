#include "TileProperty.h"
#include "XMLReaderWriter.h"
#include "util_err.h"
#include <osgDB/ObjectWrapper>

namespace game {

	TileProperty::TileProperty()
	{
	}

	TileProperty::~TileProperty()
	{
	}

	//TODO: copy constructor
	TileProperty::TileProperty(const TileProperty& other, const osg::CopyOp& copyop)
	{
	}

	const std::string& TileProperty::getTags() const {
		_tags.clear();

		int i = 0;

		for (std::set<std::string>::const_iterator it = tags.begin(); it != tags.end(); ++it) {
			_tags.append(*it);
			if (i) _tags.push_back(',');
			i++;
		}

		return _tags;
	}

	void TileProperty::modifyTags(const std::string& s, bool isRemove) {
		std::string::size_type lps = 0, lpe;

		for (;;) {
			lpe = s.find(',', lps);

			std::string ss = s.substr(lps, lpe == s.npos ? s.npos : (lpe - lps));
			if (!ss.empty()) {
				if (isRemove) tags.erase(ss);
				else tags.insert(ss);
			}

			if (lpe == s.npos) break;
			lps = lpe + 1;
		}
	}

	bool TileProperty::load(const XMLNode* node){
		//add tags
		addTags(node->getAttr("tag", std::string()));

		//check subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (false) {
			} else {
				//TODO: events
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	REG_OBJ_WRAPPER(game, TileProperty, "")
	{
		ADD_STRING_SERIALIZER(Tags, "");
	}

}
