#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"

#include <osg/Referenced>

class LevelListScreen;

typedef MyGUI::delegates::CMultiDelegate1<LevelListScreen*> EventHandle_LevelListScreenPtrLevelListScreen;

class LevelListScreen : public wraps::BaseLayout {
public:
	LevelListScreen();

	virtual ~LevelListScreen();

	/** must call this function before show */
	void initialize();

	void endMessage();

	/** Event : button on message window pressed.\n
	signature : void method(LevelListScreen* _sender)\n
	@param _sender widget that called this event
	*/
	EventHandle_LevelListScreenPtrLevelListScreen
		eventLevelListScreenAccept;

	osg::Referenced *levelOrLevelCollection; //!< the input level or level collection
	int selectedLevel; //!< the selected level

protected:
	void notifyButtonClick(MyGUI::Widget* _sender);

	void onKeyButtonPressed(MyGUI::Widget* _sender, MyGUI::KeyCode _key, MyGUI::Char _char);

	void _destroy(bool _result);

private:
	void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);
	void notifyListSelectAccept(MyGUI::MultiListBox* _sender, size_t _position);
	void notifyListMouseItemActivate(MyGUI::MultiListBox* _sender, size_t _position);

	void cmdOK_Click();

	void refreshList();

	void compareList(MyGUI::MultiListBox* _sender, size_t _column, size_t _index1, size_t _index2, bool& _less);

private:
	MyGUI::MultiListBox *lstLevel;
};
