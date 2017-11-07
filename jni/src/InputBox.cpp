#include "InputBox.h"

#include <stdio.h>

namespace MyGUI {

	InputBox::InputBox() :
		wraps::BaseLayout("InputBox.layout")
	{
		ASSIGN_WIDGET0(lblMessage);
		ASSIGN_WIDGET0(txtText);

		txtText->eventKeyButtonPressed += newDelegate(this, &InputBox::onKeyButtonPressed);

		Button *temp;
		assignWidget(temp, "cmdOK", false);
		temp->eventMouseButtonClick += newDelegate(this, &InputBox::notifyButtonClick);
		assignWidget(temp, "cmdCancel", false);
		temp->eventMouseButtonClick += newDelegate(this, &InputBox::notifyButtonClick);

		Window* window = mMainWidget->castType<Window>();
		window->eventWindowButtonPressed += newDelegate(this, &InputBox::notifyWindowButtonPressed);
		window->eventKeyButtonPressed += newDelegate(this, &InputBox::onKeyButtonPressed);
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

	void InputBox::onKeyButtonPressed(Widget* _sender, KeyCode _key, Char _char) {
		if ((_key == KeyCode::Return) || (_key == KeyCode::NumpadEnter)) {
			if (_sender == static_cast<Widget*>(txtText)) {
				if (txtText->getEditMultiLine() && !InputManager::getInstance().isControlPressed()) return;
			}
			_destroy(true);
		} else if (_key == KeyCode::Escape) {
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

		InputManager::getInstance().setKeyFocusWidget(ib->txtText);

		return ib;
	}

}
