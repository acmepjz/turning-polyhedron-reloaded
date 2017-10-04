#include "FileDialog.h"

namespace MyGUI {

	FileDialog::FileDialog() :
		wraps::BaseLayout("FileDialog.layout"),
		mSmoothShow(false)
	{
		Window *window = dynamic_cast<Window*>(mMainWidget);

		window->eventWindowButtonPressed += newDelegate(this, &FileDialog::notifyWindowButtonPressed);

		for (int i = 0, m = window->getChildCount(); i < m; i++) {
			Widget *widget = window->getChildAt(i);
			if (widget->isType<Button>()) {
				widget->eventMouseButtonClick += newDelegate(this, &FileDialog::notifyButtonClick);
			}
		}
	}

	FileDialog::~FileDialog() {

	}

	/** Set smooth message showing*/
	FileDialog* FileDialog::setSmoothShow(bool _value)
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

	FileDialog* FileDialog::setMessageModal(bool _value)
	{
		if (_value)
			InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			InputManager::getInstance().removeWidgetModal(mMainWidget);
		return this;
	}

	void FileDialog::endMessage() {
		_destroy(false);
	}

	void FileDialog::notifyButtonClick(Widget* _sender) {
		std::string _name = _sender->getName().substr(mPrefix.size()); // ???

		if (_name == "cmdCancel") {
			endMessage();
		}
	}

	void FileDialog::notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name) {
		if (_name == "close") {
			endMessage();
		}
	}

	void FileDialog::_destroy(bool _result) {
		if (_result) eventFileDialogAccept(this);

		delete this;
	}

}
