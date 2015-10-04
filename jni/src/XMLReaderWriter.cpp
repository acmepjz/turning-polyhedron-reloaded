#include "XMLReaderWriter.h"
#include "util_err.h"
#include <osg/Notify>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define U8_ENCODE(CH,OPERATION) \
	if(CH<0x80){ \
		OPERATION(CH); \
		}else if(CH<0x800){ \
		OPERATION(0xC0 | (CH>>6)); \
		OPERATION(0x80 | (CH & 0x3F)); \
		}else if(CH<0x10000){ \
		OPERATION(0xE0 | (CH>>12)); \
		OPERATION(0x80 | ((CH>>6) & 0x3F)); \
		OPERATION(0x80 | (CH & 0x3F)); \
		}else{ \
		OPERATION(0xF0 | (CH>>18)); \
		OPERATION(0x80 | ((CH>>12) & 0x3F)); \
		OPERATION(0x80 | ((CH>>6) & 0x3F)); \
		OPERATION(0x80 | (CH & 0x3F)); \
		}

#define FILE_WRITE_STRING(S) file.write(S,sizeof(S)-1)
#define ERR_UNEXPECTED_EOF() UTIL_ERR "unexpected end of file" << std::endl
#define ERR_FILE UTIL_ERR "at offset " << size_t(file.tellg()) << ": "

XMLNode::XMLNode()
	: contentType(TEXT)
{

}

XMLNode::~XMLNode(){

}

std::string XMLNode::getAttribute(const std::string& name, const std::string& default) const {
	std::map<std::string, std::string>::const_iterator it = attributes.find(name);
	if (it == attributes.end()) return default;
	else return it->second;
}

enum StringType{
	IDENTIFIER = 100,
	STRING,
	CONTENTS = XMLNode::TEXT,
	CONTENTS_CDATA = XMLNode::CDATA,
	CONTENTS_BASE64 = XMLNode::BASE64,
};

static void readWhiteSpaces(std::istream& file){
	for (;;) {
		int c = file.peek();
		switch (c) {
		case ' ':
		case '\t':
		case '\f':
		case '\v':
		case '\n':
		case '\r':
			file.get();
			continue;
		}
		return;
	}
}

static int readEscapedChar(std::istream& file){
	std::string s;

	for (;;) {
		int c = file.get();
		if (c == EOF) {
			ERR_UNEXPECTED_EOF();
			return EOF;
		}
		if (c == ';') break;
		s.push_back(c);
	}

	if (s == "lt") return '<';
	if (s == "gt") return '>';
	if (s == "amp") return '&';
	if (s == "apos") return '\'';
	if (s == "quot") return '\"';

	int ch = 0;
	size_t m = s.size();

	if (m >= 2 && (s[0] == 'x' || s[0] == 'X')) {
		for (size_t i = 1; i < m; i++) {
			char c = s[i];

			if (c >= 'A') c -= 32;

			if (c >= '0' && c <= '9') ch = (ch << 8) + (c - '0');
			else if (c >= 'A' && c <= 'F') ch = (ch << 8) + (c - ('A' - 10));
			else {
				ch = -1;
				break;
			}
		}
	} else if (m >= 1) {
		for (size_t i = 0; i < m; i++) {
			char c = s[i];

			if (c >= '0' && c <= '9') ch = ch * 10 + (c - '0');
			else {
				ch = -1;
				break;
			}
		}
	}

	if (ch >= 0 && ch < 0x110000) return ch;

	ERR_FILE "unrecognized escape sequence '" << s << "'" << std::endl;

	return EOF;
}

static void appendUTF8Char(std::string& s, int c){
	U8_ENCODE(c, s.push_back);
}

static bool readString(std::istream& file, std::string& s){
	int c = file.peek();

	if (c == '\"' || c == '\'') {
		file.get();

		int delim = c;
		for (;;) {
			c = file.get();
			if (c == EOF) {
				ERR_UNEXPECTED_EOF();
				return false;
			}

			if (c == delim) {
				return true;
			} else if (c == '&') {
				c = readEscapedChar(file);
				if (c == EOF) return false;
				appendUTF8Char(s, c);
			} else {
				s.push_back(c);
			}
		}
	}

	for (;;) {
		if (c == EOF) {
			ERR_UNEXPECTED_EOF();
			return false;
		}

		switch (c) {
		case ' ':
		case '\t':
		case '\f':
		case '\v':
		case '\n':
		case '\r':
		case '/':
		case '>':
		case '=':
		case '?':
		case '!':
		case '[':
		case ']':
		case '\"':
		case '\'':
			return true;
		case '&':
			file.get();
			c = readEscapedChar(file);
			if (c == EOF) return false;
			appendUTF8Char(s, c);
			break;
		default:
			file.get();
			s.push_back(c);
			break;
		}

		c = file.peek();
	}
}

static XMLNode* readNode(std::istream& file){
	std::vector<osg::ref_ptr<XMLNode> > stack;
	char base64char[4];
	int base64len = 0;

	//loop for reading nodes
	for (;;) {
		readWhiteSpaces(file);

		int c = file.peek();
		if (c == EOF) {
			ERR_UNEXPECTED_EOF();
			return NULL;
		} else if (c == '<') {
			// a token
			file.get(); //eat char
			c = file.peek();
			if (c == EOF) {
				ERR_UNEXPECTED_EOF();
				return NULL;
			} else if (c == '!') {
				// <!xxxxx >
				file.get(); //eat char
				std::string s;
				for (;;) {
					c = file.get();
					if (c == EOF) {
						ERR_UNEXPECTED_EOF();
						return NULL;
					}
					if (c == '>') break;
					if (s.size() < 8) s.push_back(c);
					if (s.size() == 2 && s == "--") {
						//it is comment <!-- -->
						char prev2 = 0, prev1 = 0;
						for (;;) {
							c = file.get();
							if (c == EOF) {
								ERR_UNEXPECTED_EOF();
								return NULL;
							}
							if (prev2 == '-' && prev1 == '-' && c == '>') break;
							prev2 = prev1;
							prev1 = c;
						}
						break;
					} else if (s.size() == 7 && s == "[CDATA[") {
						//it is <![CDATA[
						if (stack.empty()) {
							ERR_FILE "unexpected token outside any XML node" << std::endl;
							return NULL;
						}
						if (base64len) {
							ERR_FILE "unexpected end of base64 stream" << std::endl;
							return NULL;
						}
						XMLNode *node = stack.back().get();
						node->contentType = XMLNode::CDATA;
						char prev2 = 0, prev1 = 0;
						for (int i = 0;; i++) {
							c = file.get();
							if (c == EOF) {
								ERR_UNEXPECTED_EOF();
								return NULL;
							}
							if (prev2 == ']' && prev1 == ']' && c == '>') break;
							if (i >= 2) node->contents.push_back(prev2);
							prev2 = prev1;
							prev1 = c;
						}
						break;
					}
				}
			} else if (c == '?') {
				// <? ?>
				file.get(); //eat char

				std::string s;
				if (!readString(file, s)) return NULL;

				if (s == "base64") {
					if (stack.empty()) {
						ERR_FILE "unexpected token outside any XML node" << std::endl;
						return NULL;
					}
					stack.back()->contentType = XMLNode::BASE64;
				}

				char prev1 = 0;
				for (;;) {
					c = file.get();
					if (c == EOF) {
						ERR_UNEXPECTED_EOF();
						return NULL;
					}
					if (prev1 == '?' && c == '>') break;
					prev1 = c;
				}
			} else if (c == '/') {
				// </xxx> end of node
				if (stack.empty()) {
					ERR_FILE "unexpected token outside any XML node" << std::endl;
					return NULL;
				}
				if (base64len) {
					ERR_FILE "unexpected end of base64 stream" << std::endl;
					return NULL;
				}

				file.get(); //eat char

				std::string s;
				if (!readString(file, s)) return NULL;

				readWhiteSpaces(file);
				if ((c = file.get()) != '>') {
					ERR_FILE "'>' expected" << std::endl;
					return NULL;
				}

				if (s != stack.back()->name) {
					ERR_FILE "node name '" << s << "' mismatch; expected '" << stack.back()->name << "'" << std::endl;
					return NULL;
				}

				if (stack.size() == 1) return stack.back().release();
				stack.pop_back();
			} else {
				// a new node
				if (base64len) {
					ERR_FILE "unexpected end of base64 stream" << std::endl;
					return NULL;
				}

				osg::ref_ptr<XMLNode> node = new XMLNode;
				if (!stack.empty()) stack.back()->subNodes.push_back(node);
				stack.push_back(node);

				//read name
				if (!readString(file, node->name)) return NULL;

				//read attributes
				for (;;) {
					readWhiteSpaces(file);

					c = file.peek();
					if (c == '>') {
						// end of node header
						file.get(); //eat char
						break;
					} else if (c == '/') {
						// /> end of atom node
						file.get(); //eat char
						if ((c = file.get()) != '>') {
							ERR_FILE "'>' expected" << std::endl;
							return NULL;
						}

						if (stack.size() == 1) return stack.back().release();
						stack.pop_back();
						break;
					} else {
						// name="value" attribute
						std::string name;
						if (!readString(file, name)) return NULL;

						readWhiteSpaces(file);
						if ((c = file.get()) != '=') {
							ERR_FILE "'=' expected" << std::endl;
							return NULL;
						}

						readWhiteSpaces(file);
						std::string value;
						if (!readString(file, value)) return NULL;

						node->attributes[name] = value;
					}
				}
			}
		} else {
			// contents
			if (stack.empty()) {
				ERR_FILE "unexpected data outside any XML node" << std::endl;
				return NULL;
			}

			XMLNode *node = stack.back().get();

			if (node->contentType == XMLNode::BASE64) {
				// base 64 text
				file.get(); //eat char

				char ch;
				if (c >= 'A' && c <= 'Z') ch = c - 'A';
				else if (c >= 'a' && c <= 'z') ch = c - 'a' + 26;
				else if (c >= '0' && c <= '9') ch = c - '0' + 52;
				else if (c == '+') ch = 62;
				else if (c == '/') ch = 63;
				else if (c == '=') ch = (base64len >= 2) ? -1 : -2;
				else ch = -2;

				if (ch == -2) {
					ERR_FILE "invalid character '" << char(c) << "' in base64 stream" << std::endl;
					return NULL;
				}

				base64char[base64len++] = ch;
				if (base64len >= 4) {
					for (int i = 0; i < 4; i++) {
						if (base64char[i] == -1) {
							base64len = i;
							break;
						}
					}
					unsigned int n = 0;
					for (int k = 0; k < base64len; k++) {
						n |= ((unsigned int)(unsigned char)base64char[k]) << (18 - k * 6);
					}
					for (int k = 0; k < base64len - 1; k++) {
						node->contents.push_back(n >> (16 - k * 8));
					}
					base64len = 0;
				}
			} else {
				//normal text
				std::string s;
				std::string::size_type right = 0;
				for (;;) {
					c = file.peek();
					if (c == EOF) {
						ERR_UNEXPECTED_EOF();
						return NULL;
					}
					if (c == '<') break;
					file.get();
					if (c == '&') {
						c = readEscapedChar(file);
						if (c == EOF) return NULL;
						appendUTF8Char(s, c);
						right = s.size();
					} else {
						s.push_back(c);
					}
				}

				std::string::size_type p = s.find_last_not_of(" \t\f\v\n\r");
				if (p != std::string::npos) {
					p++;
					if (p < right) p = right;
					node->contents.append(s.begin(), s.begin() + p);
				}
			}
		}
	}

	return NULL;
}

static void writeChar(std::ostream& file, char c, StringType type){
	if (type == CONTENTS_BASE64) {
		if (c < 26) c = 'A' + c;
		else if (c < 52) c = ('a' - 26) + c;
		else if (c < 62) c = ('0' - 52) + c;
		else if (c == 62) c = '+';
		else if (c == 63) c = '/';
		file.put(c);
		return;
	}

	bool escape = false;

	if ((unsigned char)c < 32) {
		escape = true;
	} else {
		switch (c){
		case ' ':
			if (type != STRING) escape = true;
			break;
		case '<':
			FILE_WRITE_STRING("&lt;");
			return;
		case '>':
			if (type == STRING) break;
			FILE_WRITE_STRING("&gt;");
			return;
		case '&':
			FILE_WRITE_STRING("&amp;");
			return;
		case '\'':
			if (type == CONTENTS) break;
			FILE_WRITE_STRING("&apos;");
			return;
		case '\"':
			if (type == CONTENTS) break;
			FILE_WRITE_STRING("&quot;");
			return;
		case '/':
		case '=':
		case '?':
		case '!':
		case '[':
		case ']':
			if (type == IDENTIFIER) escape = true;
			break;
		}
	}

	if (escape) {
		char s[8];
		sprintf(s, "&#%d;", (unsigned char)c);
		file.write(s, strlen(s));
		return;
	}

	file.put(c);
}

static void writeIndent(std::ostream& file, int indent){
	const char tabs[16] = {
		'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
		'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
	};
	while (indent > 0) {
		file.write(tabs, indent < 16 ? indent : 16);
		indent -= 16;
	}
}

static void writeString(std::ostream& file, const std::string& s, StringType type){
	if (type == CONTENTS_CDATA) {
		char prev2 = 0, prev1 = 0;
		FILE_WRITE_STRING("<![CDATA[");
		for (size_t i = 0; i < s.size(); i++) {
			char c = s[i];
			file.put(c);
			if (prev2 == ']' && prev1 == ']' && c == '>') {
				FILE_WRITE_STRING("]]><![CDATA[");
			}
			prev2 = prev1;
			prev1 = c;
		}
		FILE_WRITE_STRING("]]>");
	} else if (type == CONTENTS_BASE64) {
		//custom XML extension :-/
		FILE_WRITE_STRING("<?base64?>");
		size_t m = s.size();
		for (size_t i = 0; i < m; i += 3) {
			size_t len = m - i;
			if (len > 3) len = 3;

			unsigned int n = 0;
			for (size_t k = 0; k < len; k++) {
				n |= ((unsigned int)(unsigned char)s[i + k]) << (16 - k * 8);
			}
			for (size_t k = 0; k < 4; k++) {
				if (k <= len) writeChar(file, (n >> (18 - k * 6)) & 63, CONTENTS_BASE64);
				else file.put('=');
			}
		}
	} else if (type == CONTENTS) {
		size_t m = s.size();
		for (size_t i = 0; i < m; i++) {
			if (i>0 && i + 1 < m && s[i] == ' '
				&& s[i - 1] != ' ' && s[i + 1] != ' ')
			{
				file.put(' ');
			} else {
				writeChar(file, s[i], type);
			}
		}
	} else {
		if (type == STRING) file.put('\"');
		for (size_t i = 0; i < s.size(); i++) {
			writeChar(file, s[i], type);
		}
		if (type == STRING) file.put('\"');
	}
}

static void writeNode(std::ostream& file, const XMLNode *node, int indent){
	if (!node) return;

	bool hasContents = node->contentType != XMLNode::TEXT
		|| !node->contents.empty()
		|| !node->subNodes.empty();

	//node name
	writeIndent(file, indent);
	file.put('<');
	writeString(file, node->name, IDENTIFIER);

	//attributes
	for (std::map<std::string, std::string>::const_iterator
		it = node->attributes.begin(); it != node->attributes.end(); it++)
	{
		file.put(' ');
		writeString(file, it->first, IDENTIFIER);
		file.put('=');
		writeString(file, it->second, STRING);
	}

	if (hasContents) {
		//end of node
		file.put('>');
		file.put('\n');

		//contents
		if (!node->contents.empty()) {
			writeIndent(file, indent + 1);
			writeString(file, node->contents, (StringType)node->contentType);
			file.put('\n');
		}

		//subnodes
		for (size_t i = 0; i < node->subNodes.size(); i++) {
			writeNode(file, node->subNodes[i].get(), indent + 1);
		}

		//end of node
		writeIndent(file, indent);
		file.put('<');
		file.put('/');
		if (node->name.empty()) file.put(' '); //add an additional space
		else writeString(file, node->name, IDENTIFIER);
	} else {
		//end of atom node
		if (node->name.empty() && node->attributes.empty()) {
			//add an additional space
			file.put(' ');
		}
		file.put('/');
	}
	file.put('>');
	file.put('\n');
}

XMLNode* XMLReaderWriter::readFile(std::istream& file){
	return readNode(file);
}

void XMLReaderWriter::writeFile(std::ostream& file, const XMLNode *node){
	FILE_WRITE_STRING("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	writeNode(file, node, 0);
}
