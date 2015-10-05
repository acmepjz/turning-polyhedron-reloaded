#pragma once

#include "util_object.h"

namespace game {

	class Interaction : public osg::Object {
	protected:
		virtual ~Interaction();
	public:
		META_Object(game, Interaction);

		Interaction();
		Interaction(const Interaction& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

		bool load(const std::string& data); //!< experimental; unimplemented
	};

}



