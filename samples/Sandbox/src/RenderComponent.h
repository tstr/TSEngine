/*
    Rendering component
*/

#pragma once

#include <vector>

#include "3D/ForwardRender.h"

namespace ts
{
    struct RenderComponent
    {
        std::vector<Renderable> items;

		RenderComponent() {}
		RenderComponent(const RenderComponent&) = delete;

		RenderComponent(RenderComponent&& rhs) :
			items(std::move(rhs.items))
		{}

		RenderComponent& operator=(RenderComponent&& rhs)
		{
			items.swap(rhs.items);
			return *this;
		}
    };
}


