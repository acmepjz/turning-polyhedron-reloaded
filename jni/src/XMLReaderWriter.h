#pragma once

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace util {

	bool getAttrFromString(const char* s, bool default);
	int getAttrFromString(const char* s, int default);
	double getAttrFromString(const char* s, double default);

	void setAttrToString(std::string& s, bool value);
	void setAttrToString(std::string& s, int value);
	void setAttrToString(std::string& s, double value);

	template <class T>
	T getAttrFromStringOsgVec(const std::string& s, const T& default) {
		T ret = default;
		std::string::size_type lps = 0;
		for (int i = 0; i < T::num_components; i++) {
			std::string::size_type lpe = s.find(',', lps);
			ret[i] = getAttrFromString(s.c_str() + lps, default[i]);
			if (lpe == s.npos) break;
			lps = lpe + 1;
		}
		return ret;
	}

	template <class T>
	T setAttrToStringOsgVec(std::string& s, const T& value) {
		for (int i = 0; i < T::num_components; i++) {
			if (i) s.push_back(',');
			setAttrToString(s, value[i]);
		}
	}

}

class XMLNode : public osg::Referenced {
public:
	enum NodeContentType{
		TEXT,
		CDATA,
		BASE64,
	};
protected:
	~XMLNode();
public:
	XMLNode();

	std::string getAttr(const std::string& name, const std::string& default) const {
		std::map<std::string, std::string>::const_iterator it = attributes.find(name);
		return (it == attributes.end()) ? default: it->second;
	}
	bool getAttr(const std::string& name, bool default) const {
		std::map<std::string, std::string>::const_iterator it = attributes.find(name);
		return (it == attributes.end()) ? default: util::getAttrFromString(it->second.c_str(), default);
	}
	int getAttr(const std::string& name, int default) const {
		std::map<std::string, std::string>::const_iterator it = attributes.find(name);
		return (it == attributes.end()) ? default: util::getAttrFromString(it->second.c_str(), default);
	}
	double getAttr(const std::string& name, double default) const {
		std::map<std::string, std::string>::const_iterator it = attributes.find(name);
		return (it == attributes.end()) ? default: util::getAttrFromString(it->second.c_str(), default);
	}

	void setAttr(const std::string& name, const std::string& value) {
		attributes[name] = value;
	}
	void setAttr(const std::string& name, bool value) {
		std::string s; util::setAttrToString(s, value); attributes[name] = value;
	}
	void setAttr(const std::string& name, int value) {
		std::string s; util::setAttrToString(s, value); attributes[name] = value;
	}
	void setAttr(const std::string& name, double value) {
		std::string s; util::setAttrToString(s, value); attributes[name] = value;
	}

	template<class T>
	T getAttrOsgVec(const std::string& name, const T& default) const {
		std::map<std::string, std::string>::const_iterator it = attributes.find(name);
		return (it == attributes.end()) ? default: util::getAttrFromStringOsgVec(it->second.c_str(), default);
	}

	template<class T>
	void setAttrOsgVec(const std::string& name, const T& value) {
		std::string s; util::setAttrToStringOsgVec(s, value); attributes[name] = value;
	}

	NodeContentType contentType;
	std::string name;
	std::string contents;
	std::map<std::string, std::string> attributes;
	std::vector<osg::ref_ptr<XMLNode> > subNodes;
};

class XMLReaderWriter {
public:
	static XMLNode* readFile(std::istream& file);
	static void writeFile(std::ostream& file, const XMLNode *node);
};
