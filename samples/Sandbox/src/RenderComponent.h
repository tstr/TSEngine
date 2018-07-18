/*
    High level graphics components
*/

#pragma once

#include <vector>

#include "3D/ForwardRender.h"

namespace ts
{
    /*
        Render component

        Encapsulates an entity that can be drawn
    */
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

    /*
        Light source component

        Encapsulates a simple light source
    */
    struct LightSourceComponent
    {

    };

    /*
        Probe component

        Encapsulates an entity that can collect visible entities
    */
    struct ProbeComponent
    {
        
    };
}