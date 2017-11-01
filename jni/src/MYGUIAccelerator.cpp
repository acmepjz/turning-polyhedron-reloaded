#include "MYGUIAccelerator.h"
#include "MYGUIManager.h"
#include <osgGA/GUIEventHandler>

MYGUIAccelerator::MYGUIAccelerator()
	: enabled(true)
	, rootWidget(NULL)
{
	if (MYGUIManager::instance) MYGUIManager::instance->accelerators.insert(this);
}

MYGUIAccelerator::~MYGUIAccelerator() {
	if (MYGUIManager::instance) MYGUIManager::instance->accelerators.erase(this);
}

void MYGUIAccelerator::addAccelerator(MyGUI::Widget* widget, bool enabled, int key, int modifier) {
	AcceleratorKey k;
	k.enabled = enabled;
	k.key = key;
	k.modifier = modifier;
	accelerators[widget] = k;
}

void MYGUIAccelerator::removeAccelerator(MyGUI::Widget* widget) {
	accelerators.erase(widget);
}

void MYGUIAccelerator::clearAccelerator() {
	accelerators.clear();
}

void MYGUIAccelerator::setAcceleratorEnabled(MyGUI::Widget* widget, bool enabled) {
	AcceleratorMap::iterator it = accelerators.find(widget);
	if (it != accelerators.end()) it->second.enabled = enabled;
}

bool MYGUIAccelerator::getAcceleratorEnabled(MyGUI::Widget* widget) const {
	AcceleratorMap::const_iterator it = accelerators.find(widget);
	if (it != accelerators.end()) return it->second.enabled;
	return false;
}

bool MYGUIAccelerator::process(int key, int modifier) {
	if (!enabled) return false;
	if (rootWidget != (MyGUI::Widget*)(-1) && rootWidget != MyGUI::InputManager::getInstance().getWidgetModal()) return false;

	for (AcceleratorMap::iterator it = accelerators.begin(); it != accelerators.end(); ++it) {
		if (it->second.enabled && it->second.key == key) {
			const int m1 = ((it->second.modifier & osgGA::GUIEventAdapter::MODKEY_CTRL) != 0 ? osgGA::GUIEventAdapter::MODKEY_CTRL : 0)
				| ((it->second.modifier & osgGA::GUIEventAdapter::MODKEY_ALT) != 0 ? osgGA::GUIEventAdapter::MODKEY_ALT : 0)
				| ((it->second.modifier & osgGA::GUIEventAdapter::MODKEY_SHIFT) != 0 ? osgGA::GUIEventAdapter::MODKEY_SHIFT : 0);
			const int m2 = ((modifier & osgGA::GUIEventAdapter::MODKEY_CTRL) != 0 ? osgGA::GUIEventAdapter::MODKEY_CTRL : 0)
				| ((modifier & osgGA::GUIEventAdapter::MODKEY_ALT) != 0 ? osgGA::GUIEventAdapter::MODKEY_ALT : 0)
				| ((modifier & osgGA::GUIEventAdapter::MODKEY_SHIFT) != 0 ? osgGA::GUIEventAdapter::MODKEY_SHIFT : 0);
			if (m1 == m2) {
				eventAcceleratorKeyPressed(this, it->first);
				return true;
			}
		}
	}

	return false;
}
