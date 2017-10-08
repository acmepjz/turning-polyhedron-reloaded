#pragma once

#include <vector>
#include <string>

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"

namespace MyGUI {

	class InputBox;

	typedef delegates::CMultiDelegate1<InputBox*> EventHandle_InputBoxPtrInputBox;

	class InputBox : public wraps::BaseLayout {
	public:
		InputBox();

		virtual ~InputBox();

		/** Set smooth message showing*/
		InputBox* setSmoothShow(bool _value);

		void endMessage();

		InputBox* setMessageModal(bool _value);

		/** Event : button on message window pressed.\n
		signature : void method(InputBox* _sender)\n
		@param _sender widget that called this event
		*/
		EventHandle_InputBoxPtrInputBox
			eventInputBoxAccept;

		EditBox* getEditBox() { return txtText; }

		UString getText() { return txtText->getOnlyText(); }

		const UString& getCaption() { return mMainWidget->castType<Window>()->getCaption(); }

		const UString& getMessage() { return lblMessage->getCaption(); }

		InputBox* setText(const UString& _value) { txtText->setOnlyText(_value); return this; }

		InputBox* setCaption(const UString& _value) { mMainWidget->castType<Window>()->setCaption(_value); return this; }

		InputBox* setMessage(const UString& _value) { lblMessage->setCaption(_value); return this; }

		static InputBox* createInputBox(
			const UString& _caption = "",
			const UString& _message = "",
			const UString& _text = "",
			bool _modal = true);

	protected:
		void notifyButtonClick(Widget* _sender);

		void onKeyButtonPressed(Widget* _sender, KeyCode _key, Char _char);

		void _destroy(bool _result);

	private:
		void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);

	private:
		bool mSmoothShow;

		TextBox *lblMessage;
		EditBox *txtText;
	};

}
