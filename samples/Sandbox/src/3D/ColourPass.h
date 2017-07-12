/*
	Colour Pass header
*/

#pragma once

#include <tsgraphics/GraphicsContext.h>

#include "MaterialInfo.h"

namespace ts
{
	/*
		Colour render pass
	*/
	class ColourPass : public GraphicsContext::Pass
	{
	public:

		ColourPass(GraphicsContext* context);

		void add(GraphicsContext::ItemID item, const SDrawCommand command);
		HDrawCmd get(GraphicsContext::ItemID item);
	};
}
