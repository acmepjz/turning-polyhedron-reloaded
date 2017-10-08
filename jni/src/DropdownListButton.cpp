/*
 * This file is modified from the source code of MyGUI <http://mygui.info/>
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "DropdownListButton.h"

namespace MyGUI
{

	const float COMBO_ALPHA_MAX  = ALPHA_MAX;
	const float COMBO_ALPHA_MIN  = ALPHA_MIN;
	const float COMBO_ALPHA_COEF = 4.0f;

	DropdownListButton::DropdownListButton() :
		mButton0(nullptr),
		mButton(nullptr),
		mList(nullptr),
		mListShow(false),
		mMaxListLength(-1),
		mItemIndex(ITEM_NONE),
		mDropMouse(false),
		mShowSmooth(false),
		mAutoCaption(true),
		mAutoSize(false),
		mListWidth(0),
		mFlowDirection(FlowDirection::TopToBottom)
	{
	}

	void DropdownListButton::initialiseOverride()
	{
		Base::initialiseOverride();

		///@wskin_child{DropdownListButton, Button, Button0}
		assignWidget(mButton0, "Button0");
		if (mButton0 != nullptr)
		{
			mButton0->eventMouseButtonClick += newDelegate(this, &DropdownListButton::notifyButtonClicked0);
		}

		///@wskin_child{DropdownListButton, Button, Button} Кнопка для выпадающего списка.
		assignWidget(mButton, "Button");
		if (mButton != nullptr)
		{
			mButton->eventMouseButtonPressed += newDelegate(this, &DropdownListButton::notifyButtonPressed);
		}

		///@wskin_child{DropdownListButton, ListBox, List} Выпадающий список.
		assignWidget(mList, "List");

		if (mList == nullptr)
		{
			std::string list_skin = getUserString("ListSkin");
			std::string list_layer = getUserString("ListLayer");

			mList = static_cast<ListBox*>(_createSkinWidget(WidgetStyle::Popup, ListBox::getClassTypeName(), list_skin, IntCoord(), Align::Default, list_layer));
		}

		if (mList != nullptr)
		{
			mList->setActivateOnClick(true);

			mList->setVisible(false);
			mList->eventKeyLostFocus += newDelegate(this, &DropdownListButton::notifyListLostFocus);
			mList->eventListSelectAccept += newDelegate(this, &DropdownListButton::notifyListSelectAccept);
			mList->eventListMouseItemActivate += newDelegate(this, &DropdownListButton::notifyListMouseItemActivate);
			mList->eventListChangePosition += newDelegate(this, &DropdownListButton::notifyListChangePosition);
		}
	}

	void DropdownListButton::shutdownOverride()
	{
		mList = nullptr;
		mButton0 = nullptr;
		mButton = nullptr;

		Base::shutdownOverride();
	}

	const UString& DropdownListButton::getCaption() {
		return mButton0->getCaption();
	}

	void DropdownListButton::setCaption(const UString& _value) {
		mButton0->setCaption(_value);
		if (mAutoSize) {
			IntSize newTextSize = mButton0->getTextSize();
			if (newTextSize.width < 4) newTextSize.width = 4;
			IntCoord oldTextSize = mButton0->getTextRegion();
			IntSize oldSize = getSize();
			oldSize.width += newTextSize.width - oldTextSize.width;
			setSize(oldSize);
		}
	}

	void DropdownListButton::setStateSelected(bool _value) {
		mButton0->setStateSelected(_value);
	}

	bool DropdownListButton::getStateSelected() const {
		return mButton0->getStateSelected();
	}

	void DropdownListButton::notifyButtonClicked0(Widget* _sender)
	{
		eventMouseButtonClick(this);
	}

	void DropdownListButton::notifyButtonPressed(Widget* _sender, int _left, int _top, MouseButton _id)
	{
		if (MouseButton::Left != _id)
			return;

		mDropMouse = true;

		if (mListShow)
			hideList();
		else
			showList();
	}

	void DropdownListButton::notifyListLostFocus(Widget* _sender, Widget* _new)
	{
		if (mDropMouse)
		{
			mDropMouse = false;
			Widget* focus = InputManager::getInstance().getMouseFocusWidget();

			// кнопка сама уберет список
			if (focus == mButton)
				return;
		}

		hideList();
	}

	void DropdownListButton::notifyListSelectAccept(ListBox* _widget, size_t _position)
	{
		mItemIndex = _position;
		if (mAutoCaption) setCaption(mItemIndex != ITEM_NONE ? mList->getItemNameAt(mItemIndex) : "");

		mDropMouse = false;
		InputManager::getInstance().setKeyFocusWidget(this);

		_resetContainer(false);

		eventComboAccept(this, mItemIndex);
	}

	void DropdownListButton::notifyListChangePosition(ListBox* _widget, size_t _position)
	{
		mItemIndex = _position;

		_resetContainer(false);

		eventComboChangePosition(this, _position);
	}

	void DropdownListButton::notifyListMouseItemActivate(ListBox* _widget, size_t _position)
	{
		mItemIndex = _position;
		if (mAutoCaption) setCaption(mItemIndex != ITEM_NONE ? mList->getItemNameAt(mItemIndex) : "");

		InputManager::getInstance().setKeyFocusWidget(this);

		_resetContainer(false);

		eventComboAccept(this, mItemIndex);
	}

	void DropdownListButton::showList()
	{
		// пустой список не показываем
		if (mList->getItemCount() == 0)
			return;

		if (mListShow)
			return;
		mListShow = true;

		IntCoord coord = calculateListPosition();
		mList->setCoord(coord);

		if (mShowSmooth)
		{
			ControllerFadeAlpha* controller = createControllerFadeAlpha(COMBO_ALPHA_MAX, COMBO_ALPHA_COEF, true);
			ControllerManager::getInstance().addItem(mList, controller);
		}
		else
		{
			mList->setVisible(true);
		}

		InputManager::getInstance().setKeyFocusWidget(mList);
	}

	void DropdownListButton::actionWidgetHide(Widget* _widget, ControllerItem* _controller)
	{
		_widget->setVisible(false);
		_widget->setEnabled(true);
	}

	void DropdownListButton::hideList()
	{
		if (!mListShow)
			return;
		mListShow = false;

		if (mShowSmooth)
		{
			ControllerFadeAlpha* controller = createControllerFadeAlpha(COMBO_ALPHA_MIN, COMBO_ALPHA_COEF, false);
			controller->eventPostAction += newDelegate(this, &DropdownListButton::actionWidgetHide);
			ControllerManager::getInstance().addItem(mList, controller);
		}
		else
		{
			mList->setVisible(false);
		}
	}

	void DropdownListButton::setIndexSelected(size_t _index)
	{
		MYGUI_ASSERT_RANGE_AND_NONE(_index, mList->getItemCount(), "DropdownListButton::setIndexSelected");
		mItemIndex = _index;
		mList->setIndexSelected(_index);
		if (_index == ITEM_NONE)
		{
			if (mAutoCaption) setCaption("");
			return;
		}
		if (mAutoCaption) setCaption(mList->getItemNameAt(_index));
	}

	void DropdownListButton::setItemNameAt(size_t _index, const UString& _name)
	{
		mList->setItemNameAt(_index, _name);
		mItemIndex = ITEM_NONE;//FIXME
		mList->setIndexSelected(mItemIndex);//FIXME
	}

	void DropdownListButton::setItemDataAt(size_t _index, Any _data)
	{
		mList->setItemDataAt(_index, _data);
		mItemIndex = ITEM_NONE;//FIXME
		mList->setIndexSelected(mItemIndex);//FIXME
	}

	void DropdownListButton::insertItemAt(size_t _index, const UString& _item, Any _data)
	{
		mList->insertItemAt(_index, _item, _data);
		mItemIndex = ITEM_NONE;//FIXME
		mList->setIndexSelected(mItemIndex);//FIXME
	}

	void DropdownListButton::removeItemAt(size_t _index)
	{
		mList->removeItemAt(_index);
		mItemIndex = ITEM_NONE;//FIXME
		mList->clearIndexSelected();//FIXME
	}

	void DropdownListButton::removeAllItems()
	{
		mItemIndex = ITEM_NONE;//FIXME
		mList->removeAllItems();//FIXME заново созданные строки криво стоят
	}

	ControllerFadeAlpha* DropdownListButton::createControllerFadeAlpha(float _alpha, float _coef, bool _enable)
	{
		ControllerItem* item = ControllerManager::getInstance().createItem(ControllerFadeAlpha::getClassTypeName());
		ControllerFadeAlpha* controller = item->castType<ControllerFadeAlpha>();

		controller->setAlpha(_alpha);
		controller->setCoef(_coef);
		controller->setEnabled(_enable);

		return controller;
	}

	size_t DropdownListButton::findItemIndexWith(const UString& _name)
	{
		return mList->findItemIndexWith(_name);
	}

	void DropdownListButton::setFlowDirection(FlowDirection _value)
	{
		mFlowDirection = _value;
	}

	IntCoord DropdownListButton::calculateListPosition()
	{
		int length = 0;
		if (mFlowDirection.isVertical())
			length = mList->getOptimalHeight();
		else
			length = mMaxListLength;

		if (mMaxListLength > 0 && length > mMaxListLength)
			length = mMaxListLength;

		// берем глобальные координаты выджета
		IntCoord coord = getAbsoluteCoord();
		// размер леера
		IntSize sizeView = mList->getParentSize();

		if (mFlowDirection == FlowDirection::TopToBottom)
		{
			if ((coord.bottom() + length) <= sizeView.height)
				coord.top += coord.height;
			else
				coord.top -= length;
			coord.height = length;
		}
		else if (mFlowDirection == FlowDirection::BottomToTop)
		{
			if ((coord.top - length) >= 0)
				coord.top -= length;
			else
				coord.top += coord.height;
			coord.height = length;
		}
		else if (mFlowDirection == FlowDirection::LeftToRight)
		{
			if ((coord.right() + length) <= sizeView.width)
				coord.left += coord.width;
			else
				coord.left -= length;
			coord.width = length;
		}
		else if (mFlowDirection == FlowDirection::RightToLeft)
		{
			if ((coord.left - length) >= 0)
				coord.left -= length;
			else
				coord.left += coord.width;
			coord.width = length;
		}

		if (mFlowDirection.isVertical()) {
			if (coord.width < mListWidth) coord.width = mListWidth;
			if (coord.right() > sizeView.width) coord.left = sizeView.width - coord.width;
			if (coord.left < 0) coord.left = 0;
		}

		return coord;
	}

	void DropdownListButton::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		/// @wproperty{DropdownListButton, FlowDirection, FlowDirection} Направление выпадения списка.
		if (_key == "FlowDirection")
			setFlowDirection(utility::parseValue<FlowDirection>(_value));

		/// @wproperty{DropdownListButton, MaxListLength, int} Максимальная высота или ширина (зависит от направления) списка в пикселях.
		else if (_key == "MaxListLength")
			setMaxListLength(utility::parseValue<int>(_value));

		/// @wproperty{DropdownListButton, SmoothShow, bool} Плавное раскрытие списка.
		else if (_key == "SmoothShow")
			setSmoothShow(utility::parseValue<bool>(_value));

		else if (_key == "AutoCaption")
			setAutoCaption(utility::parseValue<bool>(_value));

		else if (_key == "AutoSize")
			setAutoSize(utility::parseValue<bool>(_value));

		else if (_key == "ListWidth")
			setListWidth(utility::parseValue<int>(_value));

		/// @wproperty{DropdownListButton, StateSelected, bool} Set state selected.
		else if (_key == "StateSelected")
			setStateSelected(utility::parseValue<bool>(_value));

		// не коментировать
		else if (_key == "AddItem")
			addItem(_value);

		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}

		eventChangeProperty(this, _key, _value);
	}

	size_t DropdownListButton::getItemCount() const
	{
		return mList->getItemCount();
	}

	void DropdownListButton::addItem(const UString& _name, Any _data)
	{
		return insertItemAt(ITEM_NONE, _name, _data);
	}

	size_t DropdownListButton::getIndexSelected() const
	{
		return mItemIndex;
	}

	void DropdownListButton::clearIndexSelected()
	{
		setIndexSelected(ITEM_NONE);
	}

	void DropdownListButton::clearItemDataAt(size_t _index)
	{
		setItemDataAt(_index, Any::Null);
	}

	const UString& DropdownListButton::getItemNameAt(size_t _index)
	{
		return mList->getItemNameAt(_index);
	}

	void DropdownListButton::beginToItemAt(size_t _index)
	{
		mList->beginToItemAt(_index);
	}

	void DropdownListButton::beginToItemFirst()
	{
		if (getItemCount())
			beginToItemAt(0);
	}

	void DropdownListButton::beginToItemLast()
	{
		if (getItemCount())
			beginToItemAt(getItemCount() - 1);
	}

	void DropdownListButton::beginToItemSelected()
	{
		if (getIndexSelected() != ITEM_NONE)
			beginToItemAt(getIndexSelected());
	}

	void DropdownListButton::setSmoothShow(bool _value)
	{
		mShowSmooth = _value;
	}

	bool DropdownListButton::getSmoothShow() const
	{
		return mShowSmooth;
	}

	void DropdownListButton::setMaxListLength(int _value)
	{
		mMaxListLength = _value;
	}

	int DropdownListButton::getMaxListLength() const
	{
		return mMaxListLength;
	}

	FlowDirection DropdownListButton::getFlowDirection() const
	{
		return mFlowDirection;
	}

	size_t DropdownListButton::_getItemCount()
	{
		return getItemCount();
	}

	void DropdownListButton::_addItem(const MyGUI::UString& _name)
	{
		addItem(_name);
	}

	void DropdownListButton::_removeItemAt(size_t _index)
	{
		removeItemAt(_index);
	}

	void DropdownListButton::_setItemNameAt(size_t _index, const UString& _name)
	{
		setItemNameAt(_index, _name);
	}

	const UString& DropdownListButton::_getItemNameAt(size_t _index)
	{
		return getItemNameAt(_index);
	}

	void DropdownListButton::_resetContainer(bool _update)
	{
		Base::_resetContainer(_update);
		if (mList != nullptr)
			mList->_resetContainer(_update);
	}

} // namespace MyGUI
