#include "InputBox.h"

#include <stdio.h>

namespace MyGUI {

	InputBox::InputBox() :
		wraps::BaseLayout("InputBox.layout")
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

		ib->smoothShow();
		ib->setCaption(_caption)->setMessage(_message)->setText(_text);
		ib->setModal(_modal);

		return ib;
	}

}
