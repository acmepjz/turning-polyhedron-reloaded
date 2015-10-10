#pragma once

#include "util_object.h"
#include <set>

class XMLNode;

namespace game {

	/// The tile property (tags, events, etc.) in map data

	class TileProperty :
		public osg::Object
	{
	protected:
		virtual ~TileProperty();
	public:
		META_Object(game, TileProperty);

		TileProperty();
		TileProperty(const TileProperty& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		///get tags separated by ",".
		const std::string& getTags() const;

		///set tags separated by ",".
		void setTags(const std::string& s) {
			tags.clear();
			modifyTags(s, false);
		}

		///add tags separated by ",".
		void addTags(const std::string& s) {
			modifyTags(s, false);
		}

		///remove tags separated by ",".
		void removeTags(const std::string& s) {
			modifyTags(s, true);
		}

		/** add or remove tags
		\param s tags separated by ",".
		\param isRemove `false` if add tags, `true` if remove tags
		*/
		void modifyTags(const std::string& s, bool isRemove = false);

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `property`

	public:
		std::set<std::string> tags; //!< the tags (can have multiple)

	public:
		//the following properties don't save to file and is generated at runtime
		mutable std::string _tags; //!< only a temporary variable
	};

}
