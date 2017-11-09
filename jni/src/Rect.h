#pragma once

#include <osg/Vec2i>
#include <osg/Vec3i>

namespace util {

	/** A rectangle.\n
	\note The lower and upper are all INCLUSIVE.
	*/

	template <class T>
	class Rect {
	public:
		typedef typename T::value_type value_type;
		Rect()
		{
		}
		Rect(const T& lower, const T& upper)
			: lower(lower), upper(upper)
		{
		}
		Rect(const Rect& other)
			: lower(other.lower), uppert(other.upper)
		{
		}
		Rect(value_type left, value_type top, value_type right, value_type bottom)
			: lower(left, top), upper(right, bottom)
		{
		}
		Rect(value_type x0, value_type y0, value_type z0, value_type x1, value_type y1, value_type z1)
			: lower(x0, y0, z0), upper(x1, y1, z1)
		{
		}
		void set(const T& lower_, const T& upper_) {
			loewr = lower_; upper = upper_;
		}
		void set(value_type left, value_type top, value_type right, value_type bottom) {
			lower[0] = T(left, top); upper[0] = T(right, bottom);
		}
		void set(value_type x0, value_type y0, value_type z0, value_type x1, value_type y1, value_type z1) {
			lower[0] = T(x0, y0, z0); upper[0] = T(x1, y1, z1);
		}
		void expandBy(const T& p){
			for (int i = 0; i < T::num_components; i++) {
				if (lower[i] > p[i]) lower[i] = p[i];
				if (upper[i] < p[i]) upper[i] = p[i];
			}
		}
		void expandBy(value_type x, value_type y) {
			expandBy(T(x, y));
		}
		void expandBy(value_type x, value_type y, value_type z) {
			expandBy(T(x, y, z));
		}
		void expandBy(const Rect& other){
			for (int i = 0; i < T::num_components; i++) {
				if (lower[i] > other.lower[i]) lower[i] = other.lower[i];
				if (upper[i] < other.upper[i]) upper[i] = other.upper[i];
			}
		}
		value_type width() const {
			return upper[0] - lower[0];
		}
		value_type height() const {
			return upper[1] - lower[1];
		}
		T size() const{
			return upper - lower;
		}
	public:
		T lower, upper;
	};

	typedef Rect<osg::Vec2i> Rect2i;
	typedef Rect<osg::Vec3i> Rect3i;

}
