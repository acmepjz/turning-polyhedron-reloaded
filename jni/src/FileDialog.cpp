#include "FileDialog.h"

namespace MyGUI {

	FileDialog::FileDialog() :
		wraps::BaseLayout("FileDialog.layout"),
		mSmoothShow(false)
	{
		initialise();
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

}
