#pragma once

#include <osg/Object>
#include <osg/ref_ptr>
#include <string>
#include <map>

namespace game {

	class Interaction;

	/// Represents an object type (a tile or a polyhedron).

	class ObjectType : public osg::Object {
	protected:
		virtual ~ObjectType();
	public:
		META_Object(game, ObjectType);

		ObjectType();
		ObjectType(const ObjectType& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		std::string name; //!< the name, same as the id
		std::string desc; //!< the description

		/// the interaction map.
		/// use the object type of the second object (usually the tile)
		/// to determine the interaction (the first object is the polyhedron)
		std::map<std::string, osg::ref_ptr<Interaction> > interactions;
	};

	/// A map used to look up object type

	class ObjectTypeMap : public osg::Object {
	protected:
		virtual ~ObjectTypeMap();
	public:
		META_Object(game, ObjectTypeMap);

		ObjectTypeMap();
		ObjectTypeMap(const ObjectTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		ObjectType* lookup(const std::string& name);

		std::map<std::string, osg::ref_ptr<ObjectType> > map;
	};

}

