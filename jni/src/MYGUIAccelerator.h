#pragma once

#include <MYGUI/MyGUI.h>
#include <map>

#define ADDACCEL0(NAME,KEY) { MyGUI::Widget* _temp; assignWidget(_temp, NAME); \
	_accel.addAccelerator(_temp, true, osgGA::GUIEventAdapter::KEY_##KEY, \
	0); }
#define ADDACCEL1(NAME,MODIFIER,KEY) { MyGUI::Widget* _temp; assignWidget(_temp, NAME); \
	_accel.addAccelerator(_temp, true, osgGA::GUIEventAdapter::KEY_##KEY, \
	osgGA::GUIEventAdapter::MODKEY_##MODIFIER); }
#define ADDACCEL2(NAME,MODIFIER1,MODIFIER2,KEY) { MyGUI::Widget* _temp; assignWidget(_temp, NAME); \
	_accel.addAccelerator(_temp, true, osgGA::GUIEventAdapter::KEY_##KEY, \
	osgGA::GUIEventAdapter::MODKEY_##MODIFIER1 | osgGA::GUIEventAdapter::MODKEY_##MODIFIER2); }

class MYGUIAccelerator;

typedef MyGUI::delegates::CMultiDelegate2<MYGUIAccelerator*, MyGUI::Widget*> EventHandle_MYGUIAcceleratorPtrWidgetPtr;

class MYGUIAccelerator {
public:
	MYGUIAccelerator();
	~MYGUIAccelerator();

	void addAccelerator(MyGUI::Widget* widget, bool enabled, int key, int modifier);
	void removeAccelerator(MyGUI::Widget* widget);
	void clearAccelerator();
	void setAcceleratorEnabled(MyGUI::Widget* widget, bool enabled);
	bool getAcceleratorEnabled(MyGUI::Widget* widget) const;

	bool process(int key, int modifier);

public:
	struct AcceleratorKey {
		bool enabled; //!< is it enabled
		int key; //!< the key, should be osgGA::GUIEventAdapter::KEY_*
		int modifier; //!< the modifier, should be combination of osgGA::GUIEventAdapter::MODKEY_*
	};

	typedef std::map<MyGUI::Widget*, AcceleratorKey> AcceleratorMap;

	/** Event : accelerator key pressed.\n
	signature : void method(MYGUIAccelerator* _sender, MyGUI::Widget* _widget)\n
	@param _sender widget that called this event
	@param _widget the user specified widget
	*/
	EventHandle_MYGUIAcceleratorPtrWidgetPtr eventAcceleratorKeyPressed;

	/** the set of accelerator keys */
	AcceleratorMap accelerators;

	/** is it enabled */
	bool enabled;

	/** the root widget\n
	if it is NULL then this is enabled when there are no modal widget
	if it is -1 then this is always enabled
	otherwise this is enabled when the modal widget is equal to root widget
	*/
	MyGUI::Widget *rootWidget;
};
