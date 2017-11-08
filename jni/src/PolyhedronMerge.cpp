#include "PolyhedronMerge.h"
#include "Level.h"
#include "Polyhedron.h"
#include "XMLReaderWriter.h"
#include "util_err.h"
#include <osgDB/ObjectWrapper>

namespace game {

	PolyhedronMerge::PolyhedronMerge()
		: _dest(NULL)
	{
	}

	PolyhedronMerge::~PolyhedronMerge()
	{
	}

	PolyhedronMerge::PolyhedronMerge(const PolyhedronMerge& other, const osg::CopyOp& copyop)
		: osg::Object(other, copyop)
		, src(other.src)
		, dest(other.dest)
		, _dest(NULL)
	{
	}

	const std::string& PolyhedronMerge::getSource() const {
		_srcString.clear();

		int i = 0;

		for (std::set<std::string>::const_iterator it = src.begin(); it != src.end(); ++it) {
			_srcString.append(*it);
			if (i) _srcString.push_back(',');
			i++;
		}

		return _srcString;
	}

	void PolyhedronMerge::modifySource(const std::string& s, bool isRemove) {
		std::string::size_type lps = 0, lpe;

		for (;;) {
			lpe = s.find(',', lps);

			std::string ss = s.substr(lps, lpe == s.npos ? s.npos : (lpe - lps));
			if (!ss.empty()) {
				if (isRemove) src.erase(ss);
				else src.insert(ss);
			}

			if (lpe == s.npos) break;
			lps = lpe + 1;
		}
	}

	bool PolyhedronMerge::load(const XMLNode* node) {
		//add source
		addSource(node->getAttr("src", std::string()));

		//add dest
		dest = node->getAttr("dest", std::string());

		return true;
	}

	void PolyhedronMerge::init(Level* parent) {
		_src.clear();
		for (std::set<std::string>::iterator it = src.begin(); it != src.end(); ++it) {
			Level::PolyhedronMap::iterator it2 = parent->_polyhedra.find(*it);
			if (it2 == parent->_polyhedra.end()) {
				UTIL_WARN "Source polyhedron '" << (*it) << "' not found" << std::endl;
			} else {
				_src.push_back(it2->second);
			}
		}

		Level::PolyhedronMap::iterator it2 = parent->_polyhedra.find(dest);
		if (it2 == parent->_polyhedra.end()) {
			UTIL_WARN "Destination polyhedron '" << dest << "' not found" << std::endl;
			_dest = NULL;
		} else {
			_dest = it2->second;
		}
	}

	bool PolyhedronMerge::process(Level* parent) {
		if (_src.empty() || _dest == NULL) return false;

		// TODO:

		return false;
	}

	REG_OBJ_WRAPPER(game, PolyhedronMerge, "")
	{
		ADD_STRING_SERIALIZER(Source, "");
		ADD_STRING_SERIALIZER(dest, "");
	}

}
