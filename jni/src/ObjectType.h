#pragma once

#include <osg/Object>
#include <osg/ref_ptr>
#include <string>
#include <map>
#include "Interaction.h"
#include "util.h"

namespace game {

	class ObjectTypeMap;

	/// Represents an object type (a tile or a polyhedron).

	class ObjectType : public osg::Object {
	protected:
		virtual ~ObjectType();
	public:
		META_Object(game, ObjectType);

		ObjectType();
		ObjectType(const ObjectType& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		void init(ObjectTypeMap* otm);

		std::string name; //!< the name, same as the id
		std::string desc; //!< the description

		typedef std::map<std::string, osg::ref_ptr<Interaction> > InteractionMap;

		/** the interaction map.
		use the object type of the second object (usually the tile)
		to determine the interaction (the first object is the polyhedron)
		*/
		InteractionMap interactions;

	public:
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, name);
		UTIL_ADD_BYREF_GETTER_SETTER(std::string, desc);
		UTIL_ADD_BYREF_GETTER_SETTER(InteractionMap, interactions);

	public:
		//the following properties don't save to file and is generated at runtime
		typedef std::map<ObjectType*, osg::ref_ptr<Interaction> > InteractionMap2;
		InteractionMap2 _interactions;
	};

}

