#pragma once

#include <osg/Vec3i>
#include <string>

namespace util {

	bool isNumeric(const std::string s); //!< check if a string is a decimal integer without spaces
	int getCharacter(const std::string& s, std::string::size_type& lps); //!< read a character from a string, skipping white spaces

	/** a stupid function used in MapData::load and Polyhedron::load.
	\param[inout] p the position
	\param[in] size the size
	\param[in] isLast is it the last on in a run
	\param[in] c the end type, should be ',' or ';' or '|'
	*/
	void typeArrayAdvance(osg::Vec3i& p, const osg::Vec3i& size, bool isLast, int c);
}
