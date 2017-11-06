#include "BaseLayout.h"

#include <string.h>

namespace wraps {

	BaseLayout::BaseLayout() :
		mMainWidget(nullptr),
		mFrameAdvise(0)
	{
	}

	BaseLayout::BaseLayout(const std::string& _layout, MyGUI::Widget* _parent) :
		mMainWidget(nullptr),
		mFrameAdvise(0)
	{
		initialise(_layout, _parent);
	}

	void BaseLayout::initialise(const std::string& _layout, MyGUI::Widget* _parent, bool _throw, bool _createFakeWidgets)
	{
		const std::string MAIN_WINDOW1 = "_Main";
		const std::string MAIN_WINDOW2 = "Root";
		mLayoutName = _layout;

		// оборачиваем
		if (mLayoutName.empty())
		{
			mMainWidget = _parent;
			if (mMainWidget != nullptr)
			{
				mListWindowRoot.push_back(mMainWidget);
				mPrefix = FindParentPrefix(mMainWidget);
			}
		}
		// загружаем лейаут на виджет
		else
		{
			mPrefix = MyGUI::utility::toString(this, "_");
			mListWindowRoot = MyGUI::LayoutManager::getInstance().loadLayout(mLayoutName, mPrefix, _parent);

			const std::string mainName1 = mPrefix + MAIN_WINDOW1;
			const std::string mainName2 = mPrefix + MAIN_WINDOW2;
			for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
			{
				if ((*iter)->getName() == mainName1 || (*iter)->getName() == mainName2)
				{
					mMainWidget = (*iter);

					snapToParent(mMainWidget);

					break;
				}
			}

			if (mMainWidget == nullptr)
			{
				MYGUI_LOG(Warning, "Root widget with name '" << MAIN_WINDOW1 << "' or '" << MAIN_WINDOW2 << "'  not found. [" << mLayoutName << "]");
				MYGUI_ASSERT(!_throw, "No root widget. ['" << mLayoutName << "]");
				if (_createFakeWidgets)
					mMainWidget = _createFakeWidget<MyGUI::Widget>(_parent);
			}

			mMainWidget->setUserString("BaseLayoutPrefix", mPrefix);
		}
	}

	void BaseLayout::shutdown()
	{
		// disable frame enter event
		setFrameAdvise(0);

		// удаляем все классы
		for (VectorBasePtr::reverse_iterator iter = mListBase.rbegin(); iter != mListBase.rend(); ++iter)
			delete (*iter);
		mListBase.clear();

		// удаляем все рутовые виджеты
		for (MyGUI::VectorWidgetPtr::const_iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter) {
			MyGUI::Window *window = (*iter)->castType<MyGUI::Window>(false);
			if (window) {
				window->destroySmooth();
			} else {
				MyGUI::WidgetManager::getInstance().destroyWidget(*iter);
			}
		}
		mListWindowRoot.clear();
	}

	void BaseLayout::smoothShow() {
		MyGUI::Window *window = mMainWidget->castType<MyGUI::Window>(false);
		if (window) {
			window->setAlpha(MyGUI::ALPHA_MIN);
			window->setVisible(true);
			window->setVisibleSmooth(true);
		}
	}

	void BaseLayout::setModal(bool _value) {
		if (_value)
			MyGUI::InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			MyGUI::InputManager::getInstance().removeWidgetModal(mMainWidget);
	}

	void BaseLayout::frameEntered(float _frame) {
		// default implementation
		setFrameAdvise(0);
	}

	void BaseLayout::_frameEntered(float _frame) {
		frameEntered(_frame);
	}

	void BaseLayout::setFrameAdvise(int _advise) {
		if (_advise) {
			if (!mFrameAdvise) MyGUI::Gui::getInstance().eventFrameStart += MyGUI::newDelegate(this, &BaseLayout::_frameEntered);
		} else {
			if (mFrameAdvise) MyGUI::Gui::getInstance().eventFrameStart -= MyGUI::newDelegate(this, &BaseLayout::_frameEntered);
		}
		mFrameAdvise = _advise;
	}

	std::string BaseLayout::FindParentPrefix(MyGUI::Widget* _parent)
	{
		std::string prefix = _parent->getUserString("BaseLayoutPrefix");
		if (!prefix.empty())
			return prefix;
		if (_parent->getParent() != nullptr)
			return FindParentPrefix(_parent->getParent());

		return "";
	}

	void BaseLayout::snapToParent(MyGUI::Widget* _child)
	{
		if (_child->isUserString("SnapTo"))
		{
			MyGUI::Align align = MyGUI::Align::parse(_child->getUserString("SnapTo"));

			MyGUI::IntCoord coord = _child->getCoord();
			MyGUI::IntSize size = _child->getParentSize();

			if (align.isHStretch())
			{
				coord.left = 0;
				coord.width = size.width;
			} else if (align.isLeft())
			{
				coord.left = 0;
			} else if (align.isRight())
			{
				coord.left = size.width - coord.width;
			} else
			{
				coord.left = (size.width - coord.width) / 2;
			}

			if (align.isVStretch())
			{
				coord.top = 0;
				coord.height = size.height;
			} else if (align.isTop())
			{
				coord.top = 0;
			} else if (align.isBottom())
			{
				coord.top = size.height - coord.height;
			} else
			{
				coord.top = (size.height - coord.height) / 2;
			}

			_child->setCoord(coord);
		}
	}

	MyGUI::Widget* BaseLayout::_createFakeWidgetT(const std::string& _typeName, MyGUI::Widget* _parent)
	{
		if (_parent)
			return _parent->createWidgetT(_typeName, MyGUI::SkinManager::getInstance().getDefaultSkin(), MyGUI::IntCoord(), MyGUI::Align::Default);

		return MyGUI::Gui::getInstance().createWidgetT(_typeName, MyGUI::SkinManager::getInstance().getDefaultSkin(), MyGUI::IntCoord(), MyGUI::Align::Default, "");
	}

	BaseLayout::~BaseLayout()
	{
		shutdown();
	}

	std::string BaseLayout::removePrefix(const std::string& s) const {
		if (s.size() >= mPrefix.size() && (mPrefix.empty() || memcmp(s.c_str(), mPrefix.c_str(), mPrefix.size()) == 0)) {
			if (s.size() > mPrefix.size()) return s.substr(mPrefix.size());
			return std::string();
		}
		return s;
	}

}
