#pragma once

#include <osg/Object>
#include <osg/CopyOp>
#include <osg/ref_ptr>
#include <vector>
#include <string>
#include <map>

#define UTIL_ADD_CONST_GETTER(TYPE,VARNAME,PROPNAME) \
	TYPE get##PROPNAME() const { return VARNAME; }
#define UTIL_ADD_GETTER(TYPE,VARNAME,PROPNAME) \
	TYPE get##PROPNAME() { return VARNAME; }
#define UTIL_ADD_SETTER(TYPE,VARNAME,PROPNAME) \
	void set##PROPNAME(TYPE val) { VARNAME=val; }
#define UTIL_ADD_BYVAL_GETTER_SETTER(TYPE,VARNAME) \
	UTIL_ADD_CONST_GETTER(TYPE,VARNAME,VARNAME) \
	UTIL_ADD_SETTER(TYPE,VARNAME,VARNAME)
#define UTIL_ADD_BYREF_GETTER_SETTER(TYPE,VARNAME) \
	UTIL_ADD_CONST_GETTER(const TYPE&,VARNAME,VARNAME) \
	UTIL_ADD_GETTER(TYPE&,VARNAME,VARNAME) \
	UTIL_ADD_SETTER(const TYPE&,VARNAME,VARNAME)
#define UTIL_ADD_OBJ_GETTER_SETTER(TYPE,VARNAME) \
	UTIL_ADD_CONST_GETTER(const TYPE*,VARNAME,VARNAME) \
	UTIL_ADD_SETTER(TYPE*,VARNAME,VARNAME)
#define UTIL_ADD_BYVAL_GETTER_SETTER2(TYPE,VARNAME,PROPNAME) \
	UTIL_ADD_CONST_GETTER(TYPE,VARNAME,PROPNAME) \
	UTIL_ADD_SETTER(TYPE,VARNAME,PROPNAME)
#define UTIL_ADD_BYREF_GETTER_SETTER2(TYPE,VARNAME,PROPNAME) \
	UTIL_ADD_CONST_GETTER(const TYPE&,VARNAME,PROPNAME) \
	UTIL_ADD_GETTER(TYPE&,VARNAME,PROPNAME) \
	UTIL_ADD_SETTER(const TYPE&,VARNAME,PROPNAME)

#define REG_OBJ_WRAPPER(NAMESPACE,CLASS,ASSOCIATES) \
	REGISTER_OBJECT_WRAPPER(CLASS,new NAMESPACE::CLASS,NAMESPACE::CLASS,"osg::Object " ASSOCIATES #NAMESPACE "::" #CLASS)

// copied from <osgDB/Serializer>
// need to overwrite operator<< and operator>> and operator!=
#define ADD_REF_ANY_SERIALIZER(PROP, TYPE, DEF) \
    wrapper->addSerializer( new osgDB::PropByRefSerializer< MyClass, TYPE >( \
        #PROP, DEF, &MyClass::get##PROP, &MyClass::set##PROP), osgDB::BaseSerializer::RW_USER )

namespace util {

	typedef std::map<std::string, std::string> StringStringMap;

	template <class T>
	inline T* copyObj(const T* src, const osg::CopyOp& copyop) {
		if (src) return static_cast<T*>(src->clone(copyop));
		return NULL;
	}

	template <class K, class T>
	inline void copyMap(std::map<K, osg::ref_ptr<T> >& dest,
		const std::map<K, osg::ref_ptr<T> >& src,
		const osg::CopyOp& copyop,
		bool force = false)
	{
		if (force || (copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_OBJECTS)) {
			std::map<K, osg::ref_ptr<T> >::const_iterator
				it = src.begin();
			for (; it != src.end(); ++it) {
				dest[it->first] = copyObj(it->second.get(), copyop);
			}
		} else {
			dest = src;
		}
	}

	template <class T>
	inline void copyVector(std::vector<osg::ref_ptr<T> >& dest,
		const std::vector<osg::ref_ptr<T> >& src,
		const osg::CopyOp& copyop,
		bool force = false)
	{
		if (force || (copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_OBJECTS)) {
			size_t m = src.size();
			if (m > 0) {
				dest.reserve(m);
				for (size_t i = 0; i < m; i++) {
					dest.push_back(copyObj(src[i].get(), copyop));
				}
			}
		} else {
			dest = src;
		}
	}

}

