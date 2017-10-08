/*
* This file is modified from the source code of MyGUI <http://mygui.info/>
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"

namespace MyGUI
{

	struct MessageBoxStyle
	{
		enum Enum
		{
			None = MYGUI_FLAG_NONE,
			Ok = MYGUI_FLAG(0),
			Yes = MYGUI_FLAG(1),
			No = MYGUI_FLAG(2),
			Abort = MYGUI_FLAG(3),
			Retry = MYGUI_FLAG(4),
			Ignore = MYGUI_FLAG(5),
			Cancel = MYGUI_FLAG(6),
			Try = MYGUI_FLAG(7),
			Continue = MYGUI_FLAG(8),

			OkCancel = Ok | Cancel,
			YesNo = Yes | No,
			YesNoCancel = Yes | No | Cancel,
			RetryCancel = Retry | Cancel,
			AbortRetryIgnore = Abort | Retry | Ignore,

			_IndexUserButton1 = 9, // индекс первой кнопки юзера

			Button1 = MYGUI_FLAG(_IndexUserButton1),
			Button2 = MYGUI_FLAG(_IndexUserButton1 + 1),
			Button3 = MYGUI_FLAG(_IndexUserButton1 + 2),
			Button4 = MYGUI_FLAG(_IndexUserButton1 + 3),

			_CountUserButtons = 4, // колличество кнопок юзера
			_IndexIcon1 = _IndexUserButton1 + _CountUserButtons, // индекс первой иконки

			IconDefault = MYGUI_FLAG(_IndexIcon1),

			IconInfo = MYGUI_FLAG(_IndexIcon1),
			IconQuest = MYGUI_FLAG(_IndexIcon1 + 1),
			IconError = MYGUI_FLAG(_IndexIcon1 + 2),
			IconWarning = MYGUI_FLAG(_IndexIcon1 + 3),

			Icon1 = MYGUI_FLAG(_IndexIcon1),
			Icon2 = MYGUI_FLAG(_IndexIcon1 + 1),
			Icon3 = MYGUI_FLAG(_IndexIcon1 + 2),
			Icon4 = MYGUI_FLAG(_IndexIcon1 + 3),
			Icon5 = MYGUI_FLAG(_IndexIcon1 + 4),
			Icon6 = MYGUI_FLAG(_IndexIcon1 + 5),
			Icon7 = MYGUI_FLAG(_IndexIcon1 + 6),
			Icon8 = MYGUI_FLAG(_IndexIcon1 + 7)
		};

		MessageBoxStyle(Enum _value = None) :
			mValue(_value)
		{
		}

		MessageBoxStyle& operator |= (MessageBoxStyle const& _other)
		{
			mValue = Enum(int(mValue) | int(_other.mValue));
			return *this;
		}

		friend MessageBoxStyle operator | (Enum const& a, Enum const& b)
		{
			return MessageBoxStyle(Enum(int(a) | int(b)));
		}

		MessageBoxStyle operator | (Enum const& a)
		{
			return MessageBoxStyle(Enum(int(mValue) | int(a)));
		}

		friend bool operator == (MessageBoxStyle const& a, MessageBoxStyle const& b)
		{
			return a.mValue == b.mValue;
		}

		friend bool operator != (MessageBoxStyle const& a, MessageBoxStyle const& b)
		{
			return a.mValue != b.mValue;
		}

		/*friend std::ostream& operator << (std::ostream& _stream, const MessageBoxStyle&  _value)
		{
		//_stream << _value.print();
		return _stream;
		}*/

		friend std::istream& operator >> (std::istream& _stream, MessageBoxStyle&  _value);

		// возвращает индекс иконки
		size_t getIconIndex()
		{
			size_t index = 0;
			int num = mValue >> _IndexIcon1;

			while (num != 0)
			{
				if ((num & 1) == 1)
					return index;

				++index;
				num >>= 1;
			}

			return ITEM_NONE;
		}

		// возвращает индекс иконки
		size_t getButtonIndex()
		{
			size_t index = 0;
			int num = mValue;

			while (num != 0)
			{
				if ((num & 1) == 1)
					return index;

				++index;
				num >>= 1;
			}

			return ITEM_NONE;
		}

		// возвращает список кнопок
		std::vector<MessageBoxStyle> getButtons();

		typedef std::map<std::string, int> MapAlign;

		static MessageBoxStyle parse(const std::string& _value);

	private:
		static const MapAlign& getValueNames();

	private:
		Enum mValue;
	};

	class Message;

	typedef delegates::CMultiDelegate2<Message*, MessageBoxStyle> EventHandle_MessageBoxPtrMessageStyle;

	class Message :
		public wraps::BaseLayout
	{
	public:
		Message();

		Message(const std::string& _layoutName);

		virtual ~Message();

		/** Set caption text*/
		Message* setCaption(const UString& _value);

		/** Set message text*/
		Message* setMessageText(const UString& _value);

		/** Create button with specific name*/
		MessageBoxStyle addButtonName(const UString& _name);

		/** Set smooth message showing*/
		Message* setSmoothShow(bool _value);

		/** Set message icon*/
		Message* setMessageIcon(MessageBoxStyle _value);

		/** Set default button*/
		Message* setDefaultButton(MessageBoxStyle _value);

		/** Set cancel button*/
		Message* setCancelButton(MessageBoxStyle _value);

		void endMessage(MessageBoxStyle _result);

		void endMessage();

		/** Create button using MessageBoxStyle*/
		Message* setMessageButton(MessageBoxStyle _value);

		/** Set message style (button and icon)*/
		Message* setMessageStyle(MessageBoxStyle _value);

		Message* setMessageModal(bool _value);

		/** Static method for creating message with one command
			@param
				_modal if true all other GUI elements will be blocked untill message is closed
			@param
				_style any combination of flags from ViewValueInfo
			@param
				_button1 ... _button4 specific buttons names
		*/
		static Message* createMessageBox(
			const UString& _caption = "",
			const UString& _message = "",
			MessageBoxStyle _style = MessageBoxStyle::Ok | MessageBoxStyle::IconDefault,
			bool _modal = true,
			const std::string& _button1 = "",
			const std::string& _button2 = "",
			const std::string& _button3 = "",
			const std::string& _button4 = "");

	/*events:*/
		/** Event : button on message window pressed.\n
			signature : void method(tools::Message* _sender, MessageBoxStyle _result)\n
			@param _sender widget that called this event
			@param _result - id of pressed button
		*/
		EventHandle_MessageBoxPtrMessageStyle
			eventMessageBoxResult;

	protected:
		void updateSize();

		void notifyButtonClick(Widget* _sender);

		void clearButton();

		void onKeyButtonPressed(Widget* _sender, KeyCode _key, Char _char);

		void _destroyMessage(MessageBoxStyle _result);

		static UString getButtonName(MessageBoxStyle _style);

		static const char* getIconName(size_t _index);

		static const char* getButtonName(size_t _index);

		static const char* getButtonTag(size_t _index);

	private:
		void initialise();

		void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);

	private:
		IntSize mOffsetText;
		TextBox* mWidgetText;

		std::string mButtonSkin;
		std::string mButtonType;
		IntSize mButtonSize;
		IntSize mButtonOffset;

		std::vector<Button*> mVectorButton;
		MessageBoxStyle mInfoOk;
		MessageBoxStyle mInfoCancel;
		bool mSmoothShow;

		std::string mDefaultCaption;
		ImageBox* mIcon;
		int mLeftOffset1;
		int mLeftOffset2;
	};

} // namespace MyGUI
