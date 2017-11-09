#pragma once

#include "util_object.h"
#include <vector>
#include <set>

class XMLNode;

namespace game {

	class Level;
	class Polyhedron;

	/// The polyhedron merge data in map data

	class PolyhedronMerge :
		public osg::Object
	{
	protected:
		virtual ~PolyhedronMerge();
	public:
		META_Object(game, PolyhedronMerge);

		PolyhedronMerge();
		PolyhedronMerge(const PolyhedronMerge& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		///get source separated by ",".
		const std::string& getSource() const;

		///set source separated by ",".
		void setSource(const std::string& s) {
			src.clear();
			modifySource(s, false);
		}

		///add source separated by ",".
		void addSource(const std::string& s) {
			modifySource(s, false);
		}

		///remove source separated by ",".
		void removeSource(const std::string& s) {
			modifySource(s, true);
		}

		/** add or remove source
		\param s source separated by ",".
		\param isRemove `false` if add tags, `true` if remove tags
		*/
		void modifySource(const std::string& s, bool isRemove = false);

		void init(Level* parent);

		bool process(Level* parent);

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `polyhedronMerge`

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, dest);

	public:
		std::set<std::string> src; //!< the id of source polyhedra
		std::string dest; //!< the id of destination polyhedron

	public:
		//the following properties don't save to file and is generated at runtime
		mutable std::string _srcString; //!< only a temporary variable
		std::vector<Polyhedron*> _src; //!< the source polyhedra
		Polyhedron* _dest; //!< the destination polyhedron
	};

}
