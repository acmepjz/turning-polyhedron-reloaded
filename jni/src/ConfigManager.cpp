#include "ConfigManager.h"
#include "XMLReaderWriter.h"

#include <MYGUI/MyGUI.h>

#include <iostream>
#include <fstream>

ConfigManager *ConfigManager::instance = NULL;

const size_t maxRecentFiles = 10;

bool RecentFiles::add(const std::string& _fileName) {
	const int m = files.size();

	for (int i = 0; i < m; i++) {
		if (files[i] == _fileName) {
			if (i == 0) return false;

			if (owner) owner->dirty = true;

			for (; i > 0; i--) files[i] = files[i - 1];
			files[0] = _fileName;

			return true;
		}
	}

	if (owner) owner->dirty = true;

	files.insert(files.begin(), _fileName);
	while (files.size() > maxRecentFiles) files.pop_back();

	return true;
}

bool RecentFiles::remove(const std::string& _fileName) {
	const int m = files.size();
	int j = 0;
	
	for (int i = 0; i < m; i++) {
		if (files[i] == _fileName) {
			j++;
		} else if (j > 0) {
			files[i - j] = files[i];
		}
	}

	if (j > 0) {
		if (owner) owner->dirty = true;
		files.resize(files.size() - j);
		return true;
	}
	return false;
}

bool RecentFiles::remove(int index) {
	std::string s = files[index];
	return remove(s);
}

void RecentFiles::load(const XMLNode* node) {
	files.clear();

	for (int i = 0, m = node->subNodes.size(); i < m; i++) {
		const XMLNode *subnode = node->subNodes[i];
		if (subnode->name == "file") {
			std::map<std::string, std::string>::const_iterator it = subnode->attributes.find("file");
			if (it != subnode->attributes.end()) {
				files.push_back(it->second);
				if (files.size() >= maxRecentFiles) return;
			}
		}
	}
}

void RecentFiles::save(XMLNode* node) const {
	for (int i = 0, m = files.size(); i < m; i++) {
		osg::ref_ptr<XMLNode> subnode = new XMLNode;
		subnode->name = "file";
		subnode->attributes["file"] = files[i];
		node->subNodes.push_back(subnode);
	}
}

void RecentFiles::updateMenu(MyGUI::MenuControl* menu, const std::string& _name) const {
	menu->removeAllItems();

	if (files.empty()) {
		menu->addItem("(None)")->setEnabled(false);
	} else {
		for (int i = 0, m = files.size(); i < m; i++) {
			menu->addItem(files[i], MyGUI::MenuItemType::Normal, _name)->setUserString("Tag", files[i]);
		}
	}
}

ConfigManager::ConfigManager(const std::string& _fileName)
	: dirty(false)
{
	instance = this;

	fileName = _fileName;

	recentFiles.owner = this;
	recentFolders.owner = this;

	osg::ref_ptr<XMLNode> node = XMLReaderWriter::readFile(std::ifstream(fileName.c_str(), std::ios::in | std::ios::binary));
	if (!node.valid()) return;

	for (int i = 0, m = node->subNodes.size(); i < m; i++) {
		const XMLNode *subnode = node->subNodes[i];
		if (subnode->name == "recentFiles") {
			recentFiles.load(subnode);
		} else if (subnode->name == "recentFolders") {
			recentFolders.load(subnode);
		}
	}
}

void ConfigManager::save() {
	if (!dirty || fileName.empty()) return;

	osg::ref_ptr<XMLNode> node = new XMLNode;

	node->name = "config";

	osg::ref_ptr<XMLNode> subnode = new XMLNode;
	subnode->name = "recentFiles";
	recentFiles.save(subnode);
	node->subNodes.push_back(subnode);

	subnode = new XMLNode;
	subnode->name = "recentFolders";
	recentFolders.save(subnode);
	node->subNodes.push_back(subnode);

	XMLReaderWriter::writeFile(std::ofstream(fileName.c_str(), std::ios::out | std::ios::binary), node);

	dirty = false;
}

ConfigManager::~ConfigManager() {
	instance = NULL;

	save();
}
