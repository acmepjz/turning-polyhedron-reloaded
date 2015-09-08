#pragma once

#include <osg/Object>
#include <osg/ref_ptr>
#include <string>
#include <map>
#include "Interaction.h"
#include "util.h"

namespace game {

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
		typedef std::map<std::string, osg::ref_ptr<Interaction> > InteractionMap;
		InteractionMap interactions;

		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, desc);
		UTIL_ADD_BYREF_GETTER_SETTER(InteractionMap, interactions);
	};

}

