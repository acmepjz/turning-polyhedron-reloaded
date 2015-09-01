#pragma once

#include <osg/Referenced>

namespace game {

	class Interaction : public osg::Referenced {
	protected:
		virtual ~Interaction();
	public:
		Interaction();
	};

}



