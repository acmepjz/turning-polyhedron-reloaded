#pragma once

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <string>
#include <map>

namespace game {

	class Interaction;

	/// Represents an object type (a tile or a polyhedron).

	class ObjectType : public osg::Referenced {
	protected:
		virtual ~ObjectType();
	public:
		ObjectType();

		std::string name; //!< the name, same as the id
		std::string desc; //!< the description

		/// the interaction map.
		/// use the object type of the second object (usually the tile)
		/// to determine the interaction (the first object is the polyhedron)
		std::map<ObjectType*, osg::ref_ptr<Interaction> > interactions;
	};

	/// A map used to look up object type

	class ObjectTypeMap : public osg::Referenced {
	protected:
		virtual ~ObjectTypeMap();
	public:
		ObjectTypeMap();

		ObjectType* lookup(const std::string& name);

		std::map<std::string, osg::ref_ptr<ObjectType> > map;
	};

}

