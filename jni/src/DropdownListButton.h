/*
 * This file is modified from the source code of MyGUI <http://mygui.info/>
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <MYGUI/MyGUI.h>

namespace MyGUI
{

	class DropdownListButton;

	typedef delegates::CMultiDelegate2<DropdownListButton*, size_t> EventHandle_DropdownListButtonPtrSizeT;

	/** \brief @wpage{DropdownListButton}
		DropdownListButton widget description should be here.
	*/
	class DropdownListButton :
		public Widget,
		public IItemContainer
	{
		MYGUI_RTTI_DERIVED(DropdownListButton);

	public:
		DropdownListButton();

		/** Set widget caption */
		void setCaption(const UString& _value);

		/** Get widget caption */
		const UString& getCaption();

		//! Set button selected state
		void setStateSelected(bool _value);

		//! Get buton selected
		bool getStateSelected() const;

		void setAutoCaption(bool _value) { mAutoCaption = _value; }
		bool getAutoCaption() const { return mAutoCaption; }
		void setAutoSize(bool _value) { mAutoSize = _value; }
		bool getAutoSize() const { return mAutoSize; }
		void setListWidth(int _value) { mListWidth = _value; }
		int getListWidth() const { return mListWidth; }

		//------------------------------------------------------------------------------//
		// манипуляции айтемами

		//! Get number of items
		size_t getItemCount() const;

		//! Insert an item into a array at a specified position
		void insertItemAt(size_t _index, const UString& _name, Any _data = Any::Null);

		//! Add an item to the end of a array
		void addItem(const UString& _name, Any _data = Any::Null);

		//! Remove item at a specified position
		void removeItemAt(size_t _index);

		//! Remove all items
		void removeAllItems();


		//! Search item, returns the position of the first occurrence in array or ITEM_NONE if item not found
		size_t findItemIndexWith(const UString& _name);


		//------------------------------------------------------------------------------//
		// манипуляции выделениями

		//! Get index of selected item (ITEM_NONE if none selected)
		size_t getIndexSelected() const;

		//! Select specified _index
		void setIndexSelected(size_t _index);

		//! Clear item selection
		void clearIndexSelected();


		//------------------------------------------------------------------------------//
		// манипуляции данными

		//! Replace an item data at a specified position
		void setItemDataAt(size_t _index, Any _data);

		//! Clear an item data at a specified position
		void clearItemDataAt(size_t _index);

		//! Get item data from specified position
		template <typename ValueType>
		ValueType* getItemDataAt(size_t _index, bool _throw = true)
		{
			return mList->getItemDataAt<ValueType>(_index, _throw);
		}


		//------------------------------------------------------------------------------//
		// манипуляции отображением

		//! Replace an item name at a specified position
		void setItemNameAt(size_t _index, const UString& _name);

		//! Get item name from specified position
		const UString& getItemNameAt(size_t _index);


		//------------------------------------------------------------------------------//
		// манипуляции выдимостью

		//! Move all elements so specified becomes visible
		void beginToItemAt(size_t _index);

		//! Move all elements so first becomes visible
		void beginToItemFirst();

		//! Move all elements so last becomes visible
		void beginToItemLast();

		//! Move all elements so selected becomes visible
		void beginToItemSelected();


		//------------------------------------------------------------------------------------//
		// методы для управления отображением

		//! Set smooth show of list
		void setSmoothShow(bool _value);
		//! Get smooth show of list flag
		bool getSmoothShow() const;

		//! Get max list length
		void setMaxListLength(int _value);
		//! Set max list length
		int getMaxListLength() const;

		// RENAME
		//! Set direction, where drop down list appears (TopToBottom by default).
		void setFlowDirection(FlowDirection _value);
		//! Get direction, where drop down list appears.
		FlowDirection getFlowDirection() const;

		/*events:*/
		/** Event : Enter pressed in combo mode or item selected in drop down list
			and combo mode drop enabled (see void DropdownListButton::setComboModeDrop(bool _value)).\n
			signature : void method(MyGUI::DropdownListButton* _sender, size_t _index)
			@param _sender widget that called this event
			@param _index item
		*/
		EventHandle_DropdownListButtonPtrSizeT eventComboAccept;

		/** Event : Position changed.\n
			signature : void method(MyGUI::DropdownListButton* _sender, size_t _index)
			@param _sender widget that called this event
			@param _index of new position
		*/
		EventHandle_DropdownListButtonPtrSizeT eventComboChangePosition;

		/*internal:*/
		// IItemContainer impl
		virtual size_t _getItemCount();
		virtual void _addItem(const MyGUI::UString& _name);
		virtual void _removeItemAt(size_t _index);
		virtual void _setItemNameAt(size_t _index, const UString& _name);
		virtual const UString& _getItemNameAt(size_t _index);

		virtual void _resetContainer(bool _update);

	protected:
		virtual void initialiseOverride();
		virtual void shutdownOverride();

		virtual void setPropertyOverride(const std::string& _key, const std::string& _value);

	private:
		void notifyButtonClicked0(Widget* _sender);
		void notifyButtonPressed(Widget* _sender, int _left, int _top, MouseButton _id);
		void notifyListLostFocus(Widget* _sender, MyGUI::Widget* _new);
		void notifyListSelectAccept(ListBox* _widget, size_t _position);
		void notifyListMouseItemActivate(ListBox* _widget, size_t _position);
		void notifyListChangePosition(ListBox* _widget, size_t _position);

		void showList();
		void hideList();

		void actionWidgetHide(Widget* _widget, ControllerItem* _controller);

		ControllerFadeAlpha* createControllerFadeAlpha(float _alpha, float _coef, bool _enable);
		IntCoord calculateListPosition();

	private:
		Button *mButton0, *mButton;
		ListBox* mList;

		bool mListShow;
		int mMaxListLength;
		size_t mItemIndex;
		bool mDropMouse;
		bool mShowSmooth;
		bool mAutoCaption;
		bool mAutoSize;
		int mListWidth;

		FlowDirection mFlowDirection;
	};

} // namespace MyGUI
