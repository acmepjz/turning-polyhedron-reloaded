#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"
#include "MYGUIAccelerator.h"

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

	void _destroy(bool _result);

private:
	void notifyAcceleratorKeyPressed(MYGUIAccelerator* sender, MyGUI::Widget* widget);
	void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);
	void notifyListSelectAccept(MyGUI::MultiListBox* _sender, size_t _position);
	void notifyListMouseItemActivate(MyGUI::MultiListBox* _sender, size_t _position);

	virtual void frameEntered(float _frame) override;

	void cmdOK_Click();

	void refreshList();

	void compareList(MyGUI::MultiListBox* _sender, size_t _column, size_t _index1, size_t _index2, bool& _less);

private:
	MyGUI::MultiListBox *lstLevel;

	MYGUIAccelerator _accel;
};
