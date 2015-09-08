#pragma once

#include <osg/Object>
#include <osg/Node>
#include <osg/ref_ptr>
#include <string>

namespace game {

	class ObjectType;

	/// Represents a tile type.

	class TileType :
		public osg::Object
	{
	public:
		struct HitTestArea {
			HitTestArea() :
				lower(-1), upper(0)
			{
			}
			HitTestArea(int lower, int upper) :
				lower(lower), upper(upper)
			{
			}
			HitTestArea(const HitTestArea& other) :
				lower(other.lower), upper(other.upper)
			{
			}
			int lower;
			int upper;
		};
	protected:
		virtual ~TileType();
	public:
		META_Object(game, TileType);

		TileType();
		TileType(const TileType& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		std::string id; //!< id, used to find this tile type
		int index; //!< the (permanent) index (optional), used to find this tile type, 0 = no index

		std::string objType; //!< the object type

		HitTestArea blockedArea; //!< z coordinate of blocked hit test area. For example ground=(-1,0), wall=(-1,1), non-block=(0,0). Note that normal hit test area is fixed at z=0.

		std::string name; //!< the gettext'ed object name
		std::string desc; //!< the gettext'ed object description

		osg::ref_ptr<osg::Node> appearance; //!< the appearance
	};

	/// A map used to look up tile type

	class TileTypeMap : public osg::Object {
	protected:
		virtual ~TileTypeMap();
	public:
		META_Object(game, TileTypeMap);

		TileTypeMap();
		TileTypeMap(const TileTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		TileType* lookup(const std::string& idOrIndex);

		//! add a (temporary) index to a tile type with specified id
		bool addTileMapping(const std::string& id, int index);

		std::map<std::string, osg::ref_ptr<TileType> > idMap;
		std::map<int, osg::ref_ptr<TileType> > indexMap;
	};

}
