#include "LevelController.h"
#include "Level.h"

#include <osgViewer/Viewer>
#include <MYGUI/MyGUI.h>

using namespace game;

LevelController::LevelController(game::Level* lv)
	: level(lv)
{
}

bool LevelController::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
	if (!level) return false;
	if (MyGUI::InputManager::getInstancePtr() && MyGUI::InputManager::getInstancePtr()->isModalAny()) return false;

	game::Polyhedron *poly = level->getSelectedPolyhedron();
	int dir = -1;

	switch (ea.getEventType()) {
	case osgGA::GUIEventAdapter::FRAME:
		level->update();
		return false;
	case osgGA::GUIEventAdapter::KEYDOWN:
		switch (ea.getKey()) {
		case osgGA::GUIEventAdapter::KEY_Up:
			dir = 0; break;
		case osgGA::GUIEventAdapter::KEY_Left:
			dir = 1; break;
		case osgGA::GUIEventAdapter::KEY_Down:
			dir = 2; break;
		case osgGA::GUIEventAdapter::KEY_Right:
			dir = 3; break;
		case osgGA::GUIEventAdapter::KEY_Space:
			if (!level->isAnimating()) level->switchToNextPolyhedron();
			break;
		default:
			return false;
		}
		if (dir >= 0) {
			osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
			osg::Camera* camera = viewer ? viewer->getCamera() : NULL;
			if (camera) {
				osg::Vec3 eye, center, up;
				camera->getViewMatrixAsLookAt(eye, center, up);
				eye = (eye - center) ^ up;
				if (eye.x() < eye.y()) {
					if (eye.x() + eye.y() > 0) dir += 3;
				} else if (eye.x() + eye.y() > 0) dir += 2;
				else dir += 1;
			}
			const game::MoveDirection dirs[4] = { MOVE_UP, MOVE_LEFT, MOVE_DOWN, MOVE_RIGHT };
			if (!level->isAnimating() && poly) poly->move(level, dirs[dir & 3]);
		}
		break;
	default:
		return false;
	}
	return true;
}
