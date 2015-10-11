#pragma once

#include <string>

namespace util {

	bool isNumeric(const std::string s); //!< check if a string is a decimal integer without spaces
	int getCharacter(const std::string& s, std::string::size_type& lps); //!< read a character from a string, skipping white spaces

}
