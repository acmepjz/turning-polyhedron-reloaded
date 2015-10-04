#pragma once

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <string>
#include <vector>
#include <map>
#include <iostream>

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

	std::string getAttribute(const std::string& name, const std::string& default) const;

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
