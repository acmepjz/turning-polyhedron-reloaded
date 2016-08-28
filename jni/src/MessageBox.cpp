#include "MessageBox.h"

namespace MyGUI {

	std::istream& operator >> (std::istream& _stream, MessageBoxStyle&  _value)
	{
		std::string value;
		_stream >> value;
		_value = MessageBoxStyle::parse(value);
		return _stream;
	}

	// §Ó§à§Ù§Ó§â§Ñ§ë§Ñ§Ö§ä §Ú§ß§Õ§Ö§Ü§ã §Ú§Ü§à§ß§Ü§Ú
	size_t MessageBoxStyle::getIconIndex()
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

	// §Ó§à§Ù§Ó§â§Ñ§ë§Ñ§Ö§ä §Ú§ß§Õ§Ö§Ü§ã §Ú§Ü§à§ß§Ü§Ú
	size_t MessageBoxStyle::getButtonIndex()
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

	// §Ó§à§Ù§Ó§â§Ñ§ë§Ñ§Ö§ä §ã§á§Ú§ã§à§Ü §Ü§ß§à§á§à§Ü
	std::vector<MessageBoxStyle> MessageBoxStyle::getButtons()
	{
		std::vector<MessageBoxStyle> buttons;

		size_t index = 0;
		int num = mValue;
		while (index < _IndexIcon1)
		{
			if ((num & 1) == 1)
			{
				buttons.push_back(MessageBoxStyle::Enum(MYGUI_FLAG(index)));
			}

			++index;
			num >>= 1;
		}

		return buttons;
	}

	MessageBoxStyle MessageBoxStyle::parse(const std::string& _value)
	{
		MessageBoxStyle result(MessageBoxStyle::Enum(0));
		const MapAlign& map_names = result.getValueNames();
		const std::vector<std::string>& vec = utility::split(_value);
		for (size_t pos = 0; pos < vec.size(); pos++)
		{
			MapAlign::const_iterator iter = map_names.find(vec[pos]);
			if (iter != map_names.end())
			{
				result.mValue = Enum(int(result.mValue) | int(iter->second));
			} else
			{
				MYGUI_LOG(Warning, "Cannot parse type '" << vec[pos] << "'");
			}
		}
		return result;
	}

	const MessageBoxStyle::MapAlign& MessageBoxStyle::getValueNames()
	{
		static MapAlign map_names;

		if (map_names.empty())
		{
			MYGUI_REGISTER_VALUE(map_names, None);
			MYGUI_REGISTER_VALUE(map_names, Ok);
			MYGUI_REGISTER_VALUE(map_names, Yes);
			MYGUI_REGISTER_VALUE(map_names, No);
			MYGUI_REGISTER_VALUE(map_names, Abort);
			MYGUI_REGISTER_VALUE(map_names, Retry);
			MYGUI_REGISTER_VALUE(map_names, Ignore);
			MYGUI_REGISTER_VALUE(map_names, Cancel);
			MYGUI_REGISTER_VALUE(map_names, Try);
			MYGUI_REGISTER_VALUE(map_names, Continue);

			MYGUI_REGISTER_VALUE(map_names, Button1);
			MYGUI_REGISTER_VALUE(map_names, Button2);
			MYGUI_REGISTER_VALUE(map_names, Button3);
			MYGUI_REGISTER_VALUE(map_names, Button4);

			MYGUI_REGISTER_VALUE(map_names, IconDefault);

			MYGUI_REGISTER_VALUE(map_names, IconInfo);
			MYGUI_REGISTER_VALUE(map_names, IconQuest);
			MYGUI_REGISTER_VALUE(map_names, IconError);
			MYGUI_REGISTER_VALUE(map_names, IconWarning);

			MYGUI_REGISTER_VALUE(map_names, Icon1);
			MYGUI_REGISTER_VALUE(map_names, Icon2);
			MYGUI_REGISTER_VALUE(map_names, Icon3);
			MYGUI_REGISTER_VALUE(map_names, Icon4);
			MYGUI_REGISTER_VALUE(map_names, Icon5);
			MYGUI_REGISTER_VALUE(map_names, Icon6);
			MYGUI_REGISTER_VALUE(map_names, Icon7);
			MYGUI_REGISTER_VALUE(map_names, Icon8);
		}

		return map_names;
	}

	Message::Message() :
		wraps::BaseLayout("MessageBox.layout"),
		mWidgetText(nullptr),
		mInfoOk(MessageBoxStyle::None),
		mInfoCancel(MessageBoxStyle::None),
		mSmoothShow(false),
		mIcon(nullptr),
		mLeftOffset1(0),
		mLeftOffset2(0)
	{
		initialise();
	}

	Message::Message(const std::string& _layoutName) :
		wraps::BaseLayout(_layoutName),
		mWidgetText(nullptr),
		mInfoOk(MessageBoxStyle::None),
		mInfoCancel(MessageBoxStyle::None),
		mSmoothShow(false),
		mIcon(nullptr),
		mLeftOffset1(0),
		mLeftOffset2(0)
	{
		initialise();
	}

	Message::~Message()
	{
		mWidgetText = nullptr;
		mIcon = nullptr;
	}

	/** Set caption text*/
	void Message::setCaption(const UString& _value)
	{
		mMainWidget->castType<Window>()->setCaption(_value);
	}

	/** Set message text*/
	void Message::setMessageText(const UString& _value)
	{
		if (mWidgetText != nullptr)
			mWidgetText->setCaption(_value);
		updateSize();
	}

	/** Create button with specific name*/
	MessageBoxStyle Message::addButtonName(const UString& _name)
	{
		if (mVectorButton.size() >= MessageBoxStyle::_CountUserButtons)
		{
			MYGUI_LOG(Warning, "Too many buttons in message box, ignored");
			return MessageBoxStyle::None;
		}
		// §Ò§Ú§ä, §ß§à§Þ§Ö§â §Ü§ß§à§á§Ü§Ú + §ã§Þ§Ö§ë§Ö§ß§Ú§Ö §Õ§à Button1
		MessageBoxStyle info = MessageBoxStyle(MessageBoxStyle::Enum(MYGUI_FLAG(mVectorButton.size() + MessageBoxStyle::_IndexUserButton1)));

		// §Ù§Ñ§á§à§Þ§Ú§ß§Ñ§Ö§Þ §Ü§ß§à§á§Ü§Ú §Õ§Ý§ñ §à§ä§Þ§Ö§ß§í §Ú §á§à§Õ§ä§Ó§Ö§â§Ø§Õ§Ö§ß§Ú§ñ
		if (mVectorButton.empty())
			mInfoOk = info;
		mInfoCancel = info;

		Widget* widget = mMainWidget->createWidgetT(mButtonType, mButtonSkin, IntCoord(), Align::Left | Align::Bottom);
		Button* button = widget->castType<Button>();
		button->eventMouseButtonClick += newDelegate(this, &Message::notifyButtonClick);
		button->setCaption(_name);
		button->_setInternalData(info);
		mVectorButton.push_back(button);

		updateSize();
		return info;
	}

	/** Set smooth message showing*/
	void Message::setSmoothShow(bool _value)
	{
		mSmoothShow = _value;
		if (mSmoothShow)
		{
			mMainWidget->setAlpha(ALPHA_MIN);
			mMainWidget->setVisible(true);
			mMainWidget->castType<Window>()->setVisibleSmooth(true);
		}
	}

	/** Set message icon*/
	void Message::setMessageIcon(MessageBoxStyle _value)
	{
		if (nullptr == mIcon)
			return;

		if (mIcon->getItemResource() != nullptr)
		{
			mIcon->setItemName(getIconName(_value.getIconIndex()));
		} else
		{
			mIcon->setImageIndex(_value.getIconIndex());
		}

		updateSize();
	}

	void Message::endMessage(MessageBoxStyle _result)
	{
		_destroyMessage(_result);
	}

	void Message::endMessage()
	{
		_destroyMessage(mInfoCancel);
	}

	/** Create button using MessageBoxStyle*/
	void Message::setMessageButton(MessageBoxStyle _value)
	{
		clearButton();

		std::vector<MessageBoxStyle> buttons = _value.getButtons();

		for (size_t index = 0; index < buttons.size(); ++index)
		{
			// §Ü§à§â§â§Ö§Ü§ä§Ú§â§å§Ö§Þ §Ö§Ö §ß§à§Þ§Ö§â
			MessageBoxStyle info = buttons[index];

			// §Ö§ã§Ý§Ú §Ò§Ú§ä §Ö§ã§ä§î §ä§à §ã§ä§Ñ§Ó§Ú§Þ §Ü§ß§à§á§Ü§å
			addButtonName(getButtonName(info));

			// §Ó§ß§å§ä§â§Ú §Ñ§Õ§Õ §ã§Ò§â§Ñ§ã§í§Ó§Ñ§Ö§ä§ã§ñ
			mVectorButton.back()->_setInternalData(info);

			// §á§Ö§â§Ó§Ñ§ñ §Ü§ß§à§á§Ü§Ñ
			if (mVectorButton.size() == 1)
				mInfoOk = info;
			// §á§à§ã§Ý§Ö§Õ§ß§ñ§ñ §Ü§ß§à§á§Ü§Ñ
			mInfoCancel = info;
		}

		updateSize();
	}

	/** Set message style (button and icon)*/
	void Message::setMessageStyle(MessageBoxStyle _value)
	{
		setMessageButton(_value);
		setMessageIcon(_value);
	}

	void Message::setMessageModal(bool _value)
	{
		if (_value)
			InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			InputManager::getInstance().removeWidgetModal(mMainWidget);
	}

	Message* Message::createMessageBox(
		//const UString& _skinName,
		const UString& _caption,
		const UString& _message,
		MessageBoxStyle _style,
		const std::string& _layer,
		bool _modal,
		const std::string& _button1,
		const std::string& _button2,
		const std::string& _button3,
		const std::string& _button4)
	{
		Message* mess = new Message();

		mess->setCaption(_caption);
		mess->setMessageText(_message);

		mess->setSmoothShow(true);

		mess->setMessageStyle(_style);

		if (!_button1.empty())
		{
			mess->addButtonName(_button1);
			if (!_button2.empty())
			{
				mess->addButtonName(_button2);
				if (!_button3.empty())
				{
					mess->addButtonName(_button3);
				}
			}
		}

		if (_modal)
			InputManager::getInstance().addWidgetModal(mess->mMainWidget);

		return mess;
	}
	
	void Message::updateSize()
	{
		ISubWidgetText* text = nullptr;
		if (mWidgetText != nullptr)
			text = mWidgetText->getSubWidgetText();
		IntSize size = text == nullptr ? IntSize() : text->getTextSize();
		// §Þ§Ú§ß§Ú§Þ§å§Þ §Ó§í§ã§à§ä§Ñ §Ú§Ü§à§ß§Ü§Ú
		if ((nullptr != mIcon) && (mIcon->getImageIndex() != ITEM_NONE))
		{
			if (size.height < mIcon->getHeight())
				size.height = mIcon->getHeight();
			size.width += mIcon->getSize().width;
		}
		size += mOffsetText;
		size.width += 3;

		int width = ((int)mVectorButton.size() * mButtonSize.width) + (((int)mVectorButton.size() + 1) * mButtonOffset.width);
		if (size.width < width)
			size.width = width;

		int offset = (size.width - width) / 2;
		offset += mButtonOffset.width;

		size.width += mMainWidget->getWidth() - mMainWidget->getClientCoord().width;
		size.height += mMainWidget->getHeight() - mMainWidget->getClientCoord().height;

		const IntSize& view = RenderManager::getInstance().getViewSize();
		mMainWidget->setCoord((view.width - size.width) / 2, (view.height - size.height) / 2, size.width, size.height);

		if (nullptr != mIcon)
		{
			if (mWidgetText != nullptr)
			{
				if (mIcon->getImageIndex() != ITEM_NONE)
					mWidgetText->setCoord(mLeftOffset2, mWidgetText->getTop(), mWidgetText->getWidth(), mWidgetText->getHeight());
				else
					mWidgetText->setCoord(mLeftOffset1, mWidgetText->getTop(), mWidgetText->getWidth(), mWidgetText->getHeight());
			}
		}

		for (std::vector<Button*>::iterator iter = mVectorButton.begin(); iter != mVectorButton.end(); ++iter)
		{
			(*iter)->setCoord(offset, mMainWidget->getClientCoord().height - mButtonOffset.height, mButtonSize.width, mButtonSize.height);
			offset += mButtonOffset.width + mButtonSize.width;
		}
	}

	void Message::notifyButtonClick(Widget* _sender)
	{
		_destroyMessage(*_sender->_getInternalData<MessageBoxStyle>());
	}

	void Message::clearButton()
	{
		for (std::vector<Button*>::iterator iter = mVectorButton.begin(); iter != mVectorButton.end(); ++iter)
			WidgetManager::getInstance().destroyWidget(*iter);
		mVectorButton.clear();
	}

	/*void Message::onKeyButtonPressed(KeyCode _key, Char _char)
	{
	Base::onKeyButtonPressed(_key, _char);

	if ((_key == KeyCode::Return) || (_key == KeyCode::NumpadEnter))
	_destroyMessage(mInfoOk);
	else if (_key == KeyCode::Escape)
	_destroyMessage(mInfoCancel);
	}*/

	void Message::_destroyMessage(MessageBoxStyle _result)
	{
		eventMessageBoxResult(this, _result);

		delete this;
	}

	UString Message::getButtonName(MessageBoxStyle _style) const
	{
		size_t index = _style.getButtonIndex();
		const char* tag = getButtonTag(index);
		UString result = LanguageManager::getInstance().replaceTags(utility::toString("#{", tag, "}"));
		if (result == tag)
			return getButtonName(index);
		return result;
	}

	const char* Message::getIconName(size_t _index) const
	{
		static const size_t CountIcons = 4;
		static const char* IconNames[CountIcons + 1] = { "Info", "Quest", "Error", "Warning", "" };
		if (_index >= CountIcons)
			return IconNames[CountIcons];
		return IconNames[_index];
	}

	const char* Message::getButtonName(size_t _index) const
	{
		static const size_t Count = 9;
		static const char * Names[Count + 1] = { "Ok", "Yes", "No", "Abort", "Retry", "Ignore", "Cancel", "Try", "Continue", "" };
		if (_index >= Count)
			return Names[Count];
		return Names[_index];
	}

	const char* Message::getButtonTag(size_t _index) const
	{
		static const size_t Count = 9;
		static const char* Names[Count + 1] = { "MessageBox_Ok", "MessageBox_Yes", "MessageBox_No", "MessageBox_Abort", "MessageBox_Retry", "MessageBox_Ignore", "MessageBox_Cancel", "MessageBox_Try", "MessageBox_Continue", "" };
		if (_index >= Count)
			return Names[Count];
		return Names[_index];
	}
	
	void Message::initialise()
	{
		assignWidget(mWidgetText, "Text", false);
		if (mWidgetText != nullptr)
		{
			mOffsetText.set(mMainWidget->getClientCoord().width - mWidgetText->getWidth(), mMainWidget->getClientCoord().height - mWidgetText->getHeight());
			mLeftOffset2 = mLeftOffset1 = mWidgetText->getLeft();
		}

		assignWidget(mIcon, "Icon", false);
		if (mIcon != nullptr)
		{
			mLeftOffset2 = mIcon->getRight() + 3;
		}

		mButtonType = Button::getClassTypeName();

		if (mMainWidget->isUserString("ButtonSkin"))
			mButtonSkin = mMainWidget->getUserString("ButtonSkin");

		Widget* widget = nullptr;
		assignWidget(widget, "ButtonPlace", false);
		if (widget != nullptr)
		{
			mButtonOffset.set(widget->getLeft(), mMainWidget->getClientCoord().height - widget->getTop());
			widget->setVisible(false);
		}

		assignWidget(widget, "ButtonTemplate", false);
		if (widget != nullptr)
		{
			mButtonSize = widget->getSize();
		}

		Window* window = mMainWidget->castType<Window>(false);
		if (window != nullptr)
			window->eventWindowButtonPressed += newDelegate(this, &Message::notifyWindowButtonPressed);
	}

	void Message::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name)
	{
		if (_name == "close")
			endMessage();
	}

}