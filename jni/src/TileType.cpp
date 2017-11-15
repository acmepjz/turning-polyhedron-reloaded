#include "TileType.h"
#include "XMLReaderWriter.h"
#include "util_err.h"
#include "util_misc.h"
#include <osg/Node>
#include <osg/Group>
#include <osg/Notify>
#include <stdlib.h>
#include <osgDB/ObjectWrapper>

namespace game {

	TileType::TileType()
		: index(0)
		, flags(0)
		, blockedArea(-1, 0)
		, _objType(NULL)
		, _lastShape(-1)
		, _isEditMode(false)
	{
	}

	TileType::TileType(const TileType& other, const osg::CopyOp& copyop)
		: Object(other,copyop)
		, id(other.id)
		, index(other.index)
		, objType(other.objType)
		, flags(other.flags)
		, blockedArea(other.blockedArea)
		, name(other.name)
		, desc(other.desc)
		, _objType(NULL)
		, _isEditMode(false)
		, appearanceMap(util::copyObj(other.appearanceMap.get(), copyop))
	{
		util::copyVector(events, other.events, copyop, true);
	}

	TileType::~TileType()
	{
	}

	void TileType::init(ObjectTypeMap* otm, TileTypeMap* ttm){
		_objType = otm->lookup(objType);
	}

	osg::Node* TileType::getOrCreateInstance(int shape, bool isEditMode) {
		if (!_appearance.valid() || _lastShape != shape || _isEditMode != isEditMode) {
			_lastShape = shape;
			_isEditMode = isEditMode;

			gfx::Appearance *a0 = getOrCreateAppearanceMap()->lookup("", true);
			gfx::Appearance *a1 = getOrCreateAppearanceMap()->lookup(isEditMode ? "edit" : "game", true);

			if (a0 == NULL) {
				if (a1 == NULL) _appearance = new osg::Node;
				else _appearance = a1->getOrCreateInstance(shape);
			} else {
				if (a1 == NULL) _appearance = a0->getOrCreateInstance(shape);
				else {
					osg::ref_ptr<osg::Group> group = new osg::Group;
					group->addChild(a0->getOrCreateInstance(shape));
					group->addChild(a1->getOrCreateInstance(shape));
					_appearance = group;
				}
			}
		}

		return _appearance.get();
	}

	bool TileType::load(const XMLNode* node, gfx::AppearanceMap* _template){
		id = node->getAttr("id", std::string());
		if (id.empty()) {
			UTIL_WARN "object doesn't have id" << std::endl;
			return false;
		}
		index = node->getAttr("index", 0);
		objType = node->getAttr("type", std::string());

		//read flags
#define GETFLAGS(NAME,DEFAULT,FLAGS) (node->getAttr(NAME, DEFAULT) ? FLAGS : 0)
		flags = GETFLAGS("supporter", true, SUPPORTER)
			| GETFLAGS("tilt-supporter", true, TILT_SUPPORTER)
			| GETFLAGS("checkpoint", false, CHECKPOINT)
			| GETFLAGS("targetBlock", false, TARGET)
			| GETFLAGS("exitBlock", false, EXIT);
#undef GETFLAGS

		//backward compatibility code
		if (node->getAttr("non-block", false)) {
			blockedArea.set(0, 0);
		} else if (node->getAttr("blocked", false)) {
			blockedArea[0] = -1;
			blockedArea[1] = node->getAttr("block-height", 0x40000000);
		} else {
			blockedArea = node->getAttrOsgVec("blockedArea", osg::Vec2i(-1, 0));
		}

		// this is also for backward compatibility
		std::string _defaultId(node->getAttr("invisibleAtRuntime", false) ? "edit" : "");

		//check subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "name") {
				name = subnode->contents;
			} else if (subnode->name == "description") {
				desc = subnode->contents;
			} else if (subnode->name == "appearance") {
				getOrCreateAppearanceMap()->loadAppearance(subnode, _template, _defaultId, osg::Vec3(1, 1, 1));
			} else {
				int _eventType = EventHandler::convertToEventType(subnode->name);

				if (_eventType >= 0) {
					osg::ref_ptr<EventHandler> handler = new EventHandler;
					if (handler->load(subnode, _eventType)) {
						events.push_back(handler);
					}
				} else {
					UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
				}
			}
		}

		return true;
	}

	TileTypeMap::TileTypeMap(){

	}

	TileTypeMap::TileTypeMap(const TileTypeMap& other, const osg::CopyOp& copyop)
		: Object(other, copyop)
		, parent(other.parent) // FIXME: always shallow copy?
	{
		util::copyMap(idMap, other.idMap, copyop);
		util::copyMap(indexMap, other.indexMap, copyop);
	}

	TileTypeMap::~TileTypeMap(){

	}

	TileType* TileTypeMap::lookup(const std::string& idOrIndex, bool noWarning){
		if (idOrIndex.empty()) return NULL;

		if (util::isNumeric(idOrIndex)) {
			return lookupByIndex(atoi(idOrIndex.c_str()), noWarning);
		}

		return lookupById(idOrIndex, noWarning);
	}

	TileType* TileTypeMap::lookupById(const std::string& id, bool noWarning) {
		if (!id.empty()) {
			std::map<std::string, osg::ref_ptr<TileType> >::iterator it = idMap.find(id);
			if (it != idMap.end()) return it->second.get();
			else if (parent.valid()) return parent->lookupById(id, noWarning);
			if (!noWarning) UTIL_WARN "id '" << id << "' not found" << std::endl;
		}
		return NULL;
	}

	TileType* TileTypeMap::lookupByIndex(int index, bool noWarning) {
		if (index) {
			std::map<int, osg::ref_ptr<TileType> >::iterator it = indexMap.find(index);
			if (it != indexMap.end()) return it->second.get();
			else if (parent.valid()) return parent->lookupByIndex(index, noWarning);
			if (!noWarning) UTIL_WARN "index " << index << " not found" << std::endl;
		}
		return NULL;
	}

	bool TileTypeMap::add(TileType* obj){
		if (!obj || obj->id.empty()) {
			UTIL_WARN "object doesn't have id" << std::endl;
			return false;
		}

		if (lookupById(obj->id, true)) {
			UTIL_WARN "object id '" << obj->id << "' already defined, will be redefined to a new object" << std::endl;
		}

		idMap[obj->id] = obj;

		if (obj->index) {
			TileType *existing = lookupByIndex(obj->index, true);
			if (existing) {
				UTIL_WARN "object index " << obj->index << " already defined: '" << existing->id
					<< "', redefine to '" << obj->id << "'" << std::endl;
			}

			indexMap[obj->index] = obj;
		}

		return true;
	}

	bool TileTypeMap::addTileMapping(const std::string& id, int index){
		TileType *tt = lookupById(id);
		if (!tt) return false;
		return addTileMapping(tt, index);
	}

	bool TileTypeMap::addTileMapping(TileType* tile, int index){
		if (tile == NULL) return false;

		if (index == 0) {
			UTIL_WARN "index 0 invalid" << std::endl;
			return false;
		}

		TileType *existing = lookupByIndex(index, true);
		if (existing) {
			UTIL_WARN "index " << index << " already defined: '" << existing->id
				<< "', redefine to '" << tile->id << "'" << std::endl;
		}

		indexMap[index] = tile;

		return true;
	}

	void TileTypeMap::init(ObjectTypeMap* otm){
		if (parent.valid()) parent->init(otm); // ???
		for (IdMap::iterator it = idMap.begin(); it != idMap.end(); ++it) {
			it->second->init(otm, this);
		}
	}

	bool TileTypeMap::load(const XMLNode* node, gfx::AppearanceMap* _template){
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			const XMLNode* subnode = node->subNodes[i].get();

			if (subnode->name == "tileType") {
				loadTileType(subnode, _template);
			} else {
				UTIL_WARN "unrecognized node name: " << subnode->name << std::endl;
			}
		}

		return true;
	}

	bool TileTypeMap::loadTileType(const XMLNode* node, gfx::AppearanceMap* _template){
		osg::ref_ptr<TileType> tt = new TileType;
		if (tt->load(node, _template)) {
			add(tt.get());
			return true;
		} else {
			UTIL_WARN "failed to load tile type" << std::endl;
			return false;
		}
	}

	bool TileTypeMap::loadTileMapping(const XMLNode* node){
		std::string id = node->getAttr("id", std::string());
		if (id.empty()) return false;

		int index = node->getAttr("index", 0);

		return addTileMapping(id, index);
	}

#define MyClass MyClass_TileType
	REG_OBJ_WRAPPER(game, TileType, "")
	{
		ADD_STRING_SERIALIZER(id, "");
		ADD_INT_SERIALIZER(index, 0);
		ADD_STRING_SERIALIZER(objType, "");
		ADD_INT_SERIALIZER(flags, 0);
		ADD_VEC2I_SERIALIZER(blockedArea, osg::Vec2i(-1, 0));
		ADD_STRING_SERIALIZER(name, "");
		ADD_STRING_SERIALIZER(desc, "");
		ADD_OBJECT_SERIALIZER(appearanceMap, gfx::AppearanceMap, 0);
		ADD_VECTOR_SERIALIZER(events, std::vector<osg::ref_ptr<EventHandler> >, osgDB::BaseSerializer::RW_OBJECT, -1);
	}
#undef MyClass
#define MyClass MyClass_TileTypeMap
	REG_OBJ_WRAPPER(game, TileTypeMap, "")
	{
		ADD_OBJECT_SERIALIZER(parent, TileTypeMap, 0);
		ADD_MAP_SERIALIZER(idMap, TileTypeMap::IdMap, osgDB::BaseSerializer::RW_STRING, osgDB::BaseSerializer::RW_OBJECT);
		ADD_MAP_SERIALIZER(indexMap, TileTypeMap::IndexMap, osgDB::BaseSerializer::RW_INT, osgDB::BaseSerializer::RW_OBJECT);
	}
#undef MyClass

}
