#include "PolyhedronMerge.h"
#include "Level.h"
#include "MapData.h"
#include "Polyhedron.h"
#include "XMLReaderWriter.h"
#include "Rect.h"
#include "util_err.h"
#include <osgDB/ObjectWrapper>

#include <assert.h>

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
		if (_src.empty()) return false;

		// check if dest is invisible
		if (_dest == NULL || (_dest->flags & Polyhedron::VISIBLE) != 0) return false;

		const int m = _src.size();
		MapData *_map = NULL;
		util::Rect3i r;
		PolyhedronPosition::Idx idx;

		// check if src is visible, obtain the size
		for (int i = 0; i < m; i++) {
			Polyhedron *poly = _src[i];

			if (poly == NULL || (poly->flags & Polyhedron::VISIBLE) == 0) return false;

			poly->pos.getCurrentPos(poly, idx);
			util::Rect3i rr(poly->pos.pos, poly->pos.pos + idx.size - osg::Vec3i(1, 1, 1));

			if (i == 0) {
				_map = poly->pos._map;
				r = rr;
			} else {
				if (_map != poly->pos._map) return false; // TODO: adjacency support
				r.expandBy(rr);
			}
		}

		// check if it matchs the size of dest
		osg::Vec3i size = r.size() + osg::Vec3i(1, 1, 1);
		{
			osg::Vec3i s = _dest->size;
#define S1(X) ((X)[0] + (X)[1] + (X)[2])
#define S2(X) ((X)[0] * (X)[1] + ((X)[0] + (X)[1]) * (X)[2])
#define S3(X) ((X)[0] * (X)[1] * (X)[2])
			if (S1(size) != S1(s) || S2(size) != S2(s) || S3(size) != S3(s)) return false;
		}

		// populate the combined custom shape
		std::vector<unsigned char> d(size[0] * size[1] * size[2], 0);

		for (int i = 0; i < m; i++) {
			Polyhedron *poly = _src[i];
			poly->pos.getCurrentPos(poly, idx);

			for (int z = 0; z < idx.size[2]; z++) {
				int idx1 = idx.origin;
				for (int y = 0; y < idx.size[1]; y++) {
					int idx0 = idx1;
					for (int x = 0; x < idx.size[0]; x++) {
						unsigned char c = poly->customShape[poly->customShapeEnabled ? idx0 : 0];
						int idxNew = ((z + poly->pos.pos[2] - r.lower[2]) * size[1] +
							y + poly->pos.pos[1] - r.lower[1]) * size[0] +
							x + poly->pos.pos[0] - r.lower[0];
						d[idxNew] = c;
						idx0 += idx.delta[0];
					}
					idx1 += idx.delta[1];
				}
				idx.origin += idx.delta[2];
			}
		}

		// check if it matches with the custom shape of dest
		int flags = 0;
		for (int i = 0; i < PolyhedronPosition::numberOfAllPossibleFlagsForCuboid; i++) {
			PolyhedronPosition::getCurrentPos(PolyhedronPosition::allPossibleFlagsForCuboid[i], _dest, idx);
			if (idx.size == size) {
				bool match = true;
				int idxNew = 0;
				for (int z = 0; z < idx.size[2]; z++) {
					int idx1 = idx.origin;
					for (int y = 0; y < idx.size[1]; y++) {
						int idx0 = idx1;
						for (int x = 0; x < idx.size[0]; x++) {
							unsigned char c = _dest->customShape[_dest->customShapeEnabled ? idx0 : 0];
							if (d[idxNew] != c) {
								match = false;
								break;
							}
							idx0 += idx.delta[0];
							idxNew++;
						}
						idx1 += idx.delta[1];
					}
					if (!match) break;
					idx.origin += idx.delta[2];
				}

				if (match) {
					flags = PolyhedronPosition::allPossibleFlagsForCuboid[i];
					break;
				}
			}
		}

		// check if it is merged
		if (flags) {
			// hide all source polyhedron
			for (int i = 0; i < m; i++) {
				_src[i]->onRemove(parent, "teleport");
			}

			// show dest polyhedron
			// TODO: raise event & animation & check stability reason
			_dest->pos._map = _map;
			_dest->pos.pos = r.lower;
			_dest->pos.flags = flags;
			_dest->flags |= Polyhedron::VISIBLE;
			_dest->updateVisible();
			_dest->updateTransform();
			if (!_dest->valid(parent)) {
				_dest->onRemove(parent, "fall");
			}

			return true;
		}

		return false;
	}

	REG_OBJ_WRAPPER(game, PolyhedronMerge, "")
	{
		ADD_STRING_SERIALIZER(Source, "");
		ADD_STRING_SERIALIZER(dest, "");
	}

}
