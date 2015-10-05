#include "ObjectType.h"
#include "ObjectTypeMap.h"
#include "Interaction.h"
#include "util_err.h"
#include "XMLReaderWriter.h"
#include <osg/Notify>
#include <osgDB/ObjectWrapper>

namespace game {

	ObjectType::ObjectType()
	{

	}

	ObjectType::ObjectType(const ObjectType& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, name(other.name)
		, desc(other.desc)
	{
		util::copyMap(interactions, other.interactions, copyop);
	}

	ObjectType::~ObjectType(){

	}

	void ObjectType::init(ObjectTypeMap* otm){
		_interactions.clear();
		for (InteractionMap::iterator it = interactions.begin(); it != interactions.end(); ++it) {
			if (it->first.empty() || it->first == "default") {
				_interactions[NULL] = it->second;
			} else {
				ObjectType* type2 = otm->lookup(it->first);
				if (type2) _interactions[type2] = it->second;
			}
		}
	}

	bool ObjectType::loadInteractions(const XMLNode* node){
		bool ret = true;

		std::map<std::string, std::string>::const_iterator it = node->attributes.begin();
		for (; it != node->attributes.end(); ++it) {
			osg::ref_ptr<Interaction> i = new Interaction;
			if (i->load(it->second)) {
				interactions[it->first] = i;
			} else {
				ret = false;
			}
		}

		return ret;
	}

	bool ObjectType::load(const XMLNode* node){
		bool ret = true;

		name = node->getAttribute("name", "");

		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "interaction") {
				if (!loadInteractions(subnode)) ret = false;
			} else if (subnode->name == "description") {
				desc = subnode->contents;
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return ret;
	}

	REG_OBJ_WRAPPER(game, ObjectType, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_MAP_SERIALIZER(interactions, ObjectType::InteractionMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}

}
