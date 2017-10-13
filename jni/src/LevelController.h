#pragma once

#include <osgGA/GUIEventHandler>

namespace game {
	class Level;
}

/** Class to send the user input to the level (e.g. arrow keys to move polyhedron)
*/
class LevelController : public osgGA::GUIEventHandler {
public:
	LevelController(game::Level* lv = NULL);

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

public:
	game::Level* level;
};
