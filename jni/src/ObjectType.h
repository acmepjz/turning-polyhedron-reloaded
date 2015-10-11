#pragma once

#include <string>
#include <map>
#include "Interaction.h"
#include "util_object.h"

class XMLNode;

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

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `objectType`

	public:
		std::string name; //!< the name, same as the id
		std::string desc; //!< the gettext'ed description

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


	/// A map used to look up object type

	class ObjectTypeMap : public osg::Object {
	protected:
		virtual ~ObjectTypeMap();
	public:
		META_Object(game, ObjectTypeMap);

		ObjectTypeMap();
		ObjectTypeMap(const ObjectTypeMap& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool add(ObjectType* obj); //!< Add an object to map (which must has a valid id).
		ObjectType* lookup(const std::string& name); //!< Find an object with given name. If name is empty or "default" then returns NULL.

		void init();

		bool load(const XMLNode* node); //!< load from XML node, assume the node is called `objectTypes`
		bool loadObjectType(const XMLNode* node); //!< load from XML node, assume the node is called `objectType`

	public:
		typedef std::map<std::string, osg::ref_ptr<ObjectType> > IdMap;
		IdMap map;

		UTIL_ADD_BYREF_GETTER_SETTER(IdMap, map);
	};

}

