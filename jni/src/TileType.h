#pragma once

#include <osg/Referenced>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>

namespace game {

	class ObjectType;

	/// Represents a tile type.

	class TileType :
		public osg::Referenced
	{
	protected:
		virtual ~TileType();
	public:
		TileType();

		std::string id; //!< id, used to find this tile type
		int index; //!< the (permanent) index (optional), used to find this tile type, 0 = no index

		ObjectType* objType; //!< the object type, NULL = default

		std::string name; //!< the gettext'ed object name
		std::string desc; //!< the gettext'ed object description

		osg::ref_ptr<osg::Node> appearance; //!< the appearance
	};

	/// A map used to look up tile type

	class TileTypeMap : public osg::Referenced {
	protected:
		virtual ~TileTypeMap();
	public:
		TileTypeMap();

		TileType* lookup(const std::string& idOrIndex);

		//! add a (temporary) index to a tile type with specified id
		bool addTileMapping(const std::string& id, int index);

		std::map<std::string, osg::ref_ptr<TileType> > idMap;
		std::map<int, osg::ref_ptr<TileType> > indexMap;
	};

}
