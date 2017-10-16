#pragma once

#include <osg/Referenced>
#include <vector>
#include <string>
#include <map>

class XMLNode;

namespace MyGUI {
	class MenuControl;
}

class ConfigManager;

class RecentFiles {
public:
	RecentFiles() : owner(NULL) {}

	bool add(const std::string& _fileName);
	bool remove(const std::string& _fileName);
	bool remove(int index);

	void load(const XMLNode* node);
	void save(XMLNode* node) const;

	void updateMenu(MyGUI::MenuControl* _menu, const std::string& _name) const;

public:
	std::vector<std::string> files;

	ConfigManager *owner;
};

class ConfigManager : public osg::Referenced {
protected:
	~ConfigManager();

public:
	ConfigManager(const std::string& _fileName);

	void save();

public:
	RecentFiles recentFiles;
	RecentFiles recentFolders;

	std::string fileName;
	bool dirty;
};

extern ConfigManager *cfgMgr;
