#pragma once

#include <MyGUI/MyGUI.h>
#include "BaseLayout.h"

namespace MyGUI {

	class FileDialog;

	typedef delegates::CMultiDelegate1<FileDialog*> EventHandle_FileDialogPtrFileDialog;

	class FileDialog : public wraps::BaseLayout {
	public:
		FileDialog();

		virtual ~FileDialog();

		/** Set smooth message showing*/
		FileDialog* setSmoothShow(bool _value);

		void endMessage();

		FileDialog* setMessageModal(bool _value);

		/** Event : button on message window pressed.\n
		signature : void method(FileDialog* _sender)\n
		@param _sender widget that called this event
		*/
		EventHandle_FileDialogPtrFileDialog
			eventFileDialogAccept;

	protected:
		void updateSize();

		void notifyButtonClick(Widget* _sender);

		void onKeyButtonPressed(Widget* _sender, KeyCode _key, Char _char);

		void _destroy(bool _result);

	private:
		void notifyWindowButtonPressed(MyGUI::Window* _sender, const std::string& _name);

	private:
		bool mSmoothShow;
	};

}
