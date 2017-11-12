#pragma once

const char BOX_SIGNATURE[] = "\xD2\xA1\xB7\xBD\xBF\xE9\x58\x50";
const char BOX_LEV[] = "LEV";

#include <vector>
#include <string>

typedef std::string BoxFileNode;

struct BoxFileNodeArray {
	char name[4];
	std::vector<BoxFileNode> nodes;
};

class BoxFile {
public:
	bool loadFile(std::istream* f, const char* _signature = NULL, bool bSkipSignature = false);
	bool saveFile(std::ostream* f, bool IsCompress = true);
	BoxFileNodeArray* addNodeArray(const char* name = NULL);
	BoxFileNodeArray* findNodeArray(const char* name);
	const BoxFileNodeArray* findNodeArray(const char* name) const;
	void setSignature(const char* _signature);
private:
	char signature[8];
public:
	std::vector<BoxFileNodeArray> nodes;
};

namespace game {
	class LevelCollection;
}

class BoxFileLoader {
public:
	/** load level collection from box file
	\param d the BoxFile
	*/
	static game::LevelCollection* loadLevelCollection(const BoxFile& d);
};
