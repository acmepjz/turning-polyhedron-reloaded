#include "util_misc.h"

namespace util {

	bool isNumeric(const std::string s) {
		const size_t m = s.size();
		if (m == 0) return false;

		char c = s[0];
		size_t i = (c == '+' || c == '-') ? 1 : 0;

		if (i < m) {
			for (; i < m; i++) {
				c = s[i];
				if (c<'0' || c>'9') return false;
			}
			return true;
		}

		return false;
	}

	int getCharacter(const std::string& s, std::string::size_type& lps) {
		std::string::size_type m = s.size();
		for (; lps < m;) {
			unsigned char c = s[lps++];
			switch (c) {
			case ' ':
			case '\t':
			case '\f':
			case '\v':
			case '\n':
			case '\r':
				continue;
			}
			return c;
		}
		return EOF;
	}

}
