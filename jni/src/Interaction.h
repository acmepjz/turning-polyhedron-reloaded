#pragma once

#include <osg/Object>

namespace game {

	class Interaction : public osg::Object {
	protected:
		virtual ~Interaction();
	public:
		META_Object(game, Interaction);

		Interaction();
		Interaction(const Interaction& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
	};

}



