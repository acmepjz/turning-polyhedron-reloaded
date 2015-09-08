#pragma once

#include <osg/CopyOp>
#include <osg/ref_ptr>
#include <vector>
#include <map>

namespace util {

	template <class K, class T>
	inline void copyMap(std::map<K, osg::ref_ptr<T> >& dest,
		const std::map<K, osg::ref_ptr<T> >& src,
		const osg::CopyOp& copyop)
	{
		if (copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_OBJECTS) {
			std::map<K, osg::ref_ptr<T> >::const_iterator
				it = src.begin();
			for (; it != src.end(); ++it) {
				dest[it->first] = new T(*it->second, copyop);
			}
		} else {
			dest = src;
		}
	}

	template <class T>
	inline void copyVector(std::vector<osg::ref_ptr<T> >& dest,
		const std::vector<osg::ref_ptr<T> >& src,
		const osg::CopyOp& copyop)
	{
		if (copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_OBJECTS) {
			size_t m = src.size();
			if (m > 0) {
				dest.reserve(m);
				for (size_t i = 0; i < m; i++) {
					dest.push_back(new T(*src[i], copyop));
				}
			}
		} else {
			dest = src;
		}
	}

}

