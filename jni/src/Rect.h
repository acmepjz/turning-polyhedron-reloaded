#pragma once

namespace util {

	/// A rectangle.

	template <class T>
	class Rect {
	public:
		Rect() : left(0), top(0), right(0), bottom(0)
		{
		}
		Rect(T left, T top, T right, T bototm)
			: left(left), top(top), right(right), bottom(bottom)
		{
		}
		Rect(const Rect& other)
			: left(other.left), top(other.top), right(other.right), bottom(other.bottom)
		{
		}
		void set(T left_, T top_, T right_, T bottom_){
			left = left_; top = top_; right = right_; bottom = bottom_;
		}
		void expandBy(T x, T y){
			if (left > x) left = x;
			if (top > y) top = y;
			if (right < x) right = x;
			if (bottom < y) bottom = y;
		}
		void expandBy(const Rect& other){
			if (left > other.left) left = other.left;
			if (top > other.top) top = other.top;
			if (right < other.right) right = other.right;
			if (bottom < other.bottom) bottom = other.bottom;
		}
		T width() const{
			return right - left;
		}
		T height() const{
			return bottom - top;
		}
	public:
		T left, top, right, bottom;
	};

}
