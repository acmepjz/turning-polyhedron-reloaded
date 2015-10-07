#include "ObjectType.h"
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

		name = node->getAttr("name", "");

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

	ObjectTypeMap::ObjectTypeMap(){

	}

	ObjectTypeMap::ObjectTypeMap(const ObjectTypeMap& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
	{
		util::copyMap(map, other.map, copyop);
	}

	ObjectTypeMap::~ObjectTypeMap(){

	}

	bool ObjectTypeMap::add(ObjectType* obj){
		if (!obj || obj->name.empty()) {
			UTIL_ERR "object doesn't have id" << std::endl;
			return false;
		}

		IdMap::iterator it = map.find(obj->name);
		if (it != map.end()) {
			UTIL_WARN "object name '" << it->first << "' already defined, will be redefined to a new object" << std::endl;
		}

		map[obj->name] = obj;
		return true;
	}

	ObjectType* ObjectTypeMap::lookup(const std::string& name){
		if (name.empty() || name == "default") return NULL;
		IdMap::iterator it = map.find(name);
		if (it == map.end()) {
			UTIL_WARN "name '" << name << "' not found" << std::endl;
			return NULL;
		}
		return it->second.get();
	}

	void ObjectTypeMap::init(){
		for (IdMap::iterator it = map.begin(); it != map.end(); ++it) {
			it->second->init(this);
		}
	}

	bool ObjectTypeMap::load(const XMLNode* node){
		bool ret = true;

		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "objectType") {
				osg::ref_ptr<ObjectType> ot = new ObjectType;
				if (!ot->load(subnode) || !add(ot.get())) {
					ret = false;
				}
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return ret;
	}

#define MyClass MyClass_ObjectType
	REG_OBJ_WRAPPER(game, ObjectType, "")
	{
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_MAP_SERIALIZER(interactions, ObjectType::InteractionMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}
#undef MyClass
#define MyClass MyClass_ObjectTypeMap
	REG_OBJ_WRAPPER(game, ObjectTypeMap, "")
	{
		ADD_MAP_SERIALIZER(map, ObjectTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
	}
#undef MyClass

}
