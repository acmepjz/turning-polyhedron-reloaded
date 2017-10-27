#include "LevelListScreen.h"
#include "Level.h"
#include "LevelCollection.h"

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>

#include <stdio.h>
#include <string.h>

using namespace MyGUI;

LevelListScreen::LevelListScreen() :
	wraps::BaseLayout("LevelListScreen.layout"),
	mSmoothShow(false),
	levelOrLevelCollection(0),
	selectedLevel(-1)
{
}

void LevelListScreen::initialize() {
	Window *window = dynamic_cast<Window*>(mMainWidget);

	window->eventWindowButtonPressed += newDelegate(this, &LevelListScreen::notifyWindowButtonPressed);

	assignWidget(lstLevel, "lstLevel", false);
	lstLevel->addColumn("Level number", 128);
	lstLevel->addColumn("Level name");
	lstLevel->setColumnResizingPolicyAt(0, ResizingPolicy::Fixed);
	lstLevel->setColumnResizingPolicyAt(1, ResizingPolicy::Fill);
	lstLevel->setColumnSkinLineAt(0, "ListBoxItemL");
	lstLevel->setColumnSkinLineAt(1, "ListBoxItemR");

	lstLevel->requestOperatorLess2 = newDelegate(this, &LevelListScreen::compareList);

	lstLevel->sortByColumn(0);

	lstLevel->eventListMouseItemActivate += newDelegate(this, &LevelListScreen::notifyListMouseItemActivate);
	lstLevel->eventListSelectAccept += newDelegate(this, &LevelListScreen::notifyListSelectAccept);

	{
		Button *temp;
		assignWidget(temp, "cmdOK", false);
		temp->eventMouseButtonClick += newDelegate(this, &LevelListScreen::notifyButtonClick);
		assignWidget(temp, "cmdCancel", false);
		temp->eventMouseButtonClick += newDelegate(this, &LevelListScreen::notifyButtonClick);
	}

	refreshList();
}

void LevelListScreen::refreshList() {
	lstLevel->removeAllItems();

	char s[32];

	game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelOrLevelCollection);
	if (lc) {
		for (size_t i = 0; i < lc->levels.size(); i++) {
			sprintf(s, "%d", i + 1);
			lstLevel->addItem(s);
			lstLevel->setSubItemNameAt(1, i, lc->levels[i]->name);
		}
		if (selectedLevel >= 0 && selectedLevel < (int)lc->levels.size()) {
			lstLevel->setIndexSelected(selectedLevel);
		}
		return;
	}

	game::Level *lv = dynamic_cast<game::Level*>(levelOrLevelCollection);
	if (lv) {
		lstLevel->addItem("1");
		lstLevel->setSubItemNameAt(1, 0, lv->name);
		lstLevel->setIndexSelected(0);
		return;
	}
}

void LevelListScreen::compareList(MyGUI::MultiListBox* _sender, size_t _column, size_t _index1, size_t _index2, bool& _less) {
	switch (_column) {
	case 1: // sort by name
	{
		game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelOrLevelCollection);
		if (lc) {
			const std::string& name1 = lc->levels[_index1]->name;
			const std::string& name2 = lc->levels[_index2]->name;

			std::string& name1b = osgDB::convertToLowerCase(name1);
			std::string& name2b = osgDB::convertToLowerCase(name2);

			int ret = strcmp(name1b.c_str(), name2b.c_str());
			if (ret == 0) ret = strcmp(name1.c_str(), name2.c_str());

			if (ret) {
				_less = ret < 0;
				return;
			}
		}
	}
		// fall through
	default: // sort by index
		_less = _index1 < _index2;
		break;
	}
}

LevelListScreen::~LevelListScreen() {
}

LevelListScreen* LevelListScreen::setSmoothShow(bool _value)
{
	mSmoothShow = _value;
	if (mSmoothShow)
	{
		mMainWidget->setAlpha(ALPHA_MIN);
		mMainWidget->setVisible(true);
		mMainWidget->castType<Window>()->setVisibleSmooth(true);
	}
	return this;
}

LevelListScreen* LevelListScreen::setMessageModal(bool _value)
{
	if (_value)
		InputManager::getInstance().addWidgetModal(mMainWidget);
	else
		InputManager::getInstance().removeWidgetModal(mMainWidget);
	return this;
}

void LevelListScreen::endMessage() {
	_destroy(false);
}

void LevelListScreen::notifyButtonClick(Widget* _sender) {
	std::string _name = removePrefix(_sender->getName());

	if (_name == "cmdCancel") {
		endMessage();
	} else if (_name == "cmdOK") {
		cmdOK_Click();
	}
}

void LevelListScreen::cmdOK_Click() {
	selectedLevel = lstLevel->getIndexSelected();

	game::LevelCollection *lc = dynamic_cast<game::LevelCollection*>(levelOrLevelCollection);
	if (lc) {
		if (selectedLevel >= 0 && selectedLevel < (int)lc->levels.size()) {
			_destroy(true);
		}
		return;
	}

	game::Level *lv = dynamic_cast<game::Level*>(levelOrLevelCollection);
	if (lv) {
		if (selectedLevel == 0) {
			_destroy(false); //??
		}
		return;
	}
}

void LevelListScreen::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name) {
	if (_name == "close") {
		endMessage();
	}
}

void LevelListScreen::notifyListSelectAccept(MultiListBox* _sender, size_t _position) {
	cmdOK_Click();
}

void LevelListScreen::notifyListMouseItemActivate(MultiListBox* _sender, size_t _position) {
	// do nothing for now
}

void LevelListScreen::_destroy(bool _result) {
	if (_result) {
		eventLevelListScreenAccept(this);
	}

	delete this;
}
