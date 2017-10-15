#include "InputBox.h"

#include <stdio.h>

namespace MyGUI {

	InputBox::InputBox() :
		wraps::BaseLayout("InputBox.layout"),
		mSmoothShow(false)
	{
		assignWidget(lblMessage, "lblMessage", false);
		assignWidget(txtText, "txtText", false);

		Button *temp;
		assignWidget(temp, "cmdOK", false);
		temp->eventMouseButtonClick += newDelegate(this, &InputBox::notifyButtonClick);
		assignWidget(temp, "cmdCancel", false);
		temp->eventMouseButtonClick += newDelegate(this, &InputBox::notifyButtonClick);
	}

	InputBox::~InputBox() {
	}

	InputBox* InputBox::setSmoothShow(bool _value)
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

	InputBox* InputBox::setMessageModal(bool _value)
	{
		if (_value)
			InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			InputManager::getInstance().removeWidgetModal(mMainWidget);
		return this;
	}

	void InputBox::endMessage() {
		_destroy(false);
	}

	void InputBox::notifyButtonClick(Widget* _sender) {
		std::string _name = removePrefix(_sender->getName());

		if (_name == "cmdCancel") {
			endMessage();
		} else if (_name == "cmdOK") {
			_destroy(true);
		}
	}

	void InputBox::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name) {
		if (_name == "close") {
			endMessage();
		}
	}

	void InputBox::_destroy(bool _result) {
		if (_result) {
			eventInputBoxAccept(this);
		}

		delete this;
	}

	InputBox* InputBox::createInputBox(
		const UString& _caption,
		const UString& _message,
		const UString& _text,
		bool _modal)
	{
		InputBox *ib = new InputBox();

		return ib->setSmoothShow(true)->setCaption(_caption)->setMessage(_message)->setText(_text)->setMessageModal(_modal);
	}

}
