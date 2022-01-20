#pragma once

#include "../Common/Logger.h"

namespace ui
{
	class Overlay : public Logger
	{
	public:
		Overlay();

		bool Init();
		bool Update();
	};

}