/*
    Renderable List
*/

#pragma once

#include <tscore/maths.h>

#include "Renderable.h"

namespace ts
{
    class RenderableList
    {
    public:

        struct Item
        {
            Matrix transform;
            const Renderable* item;

            Item() {}
            Item(const Matrix& transform, const Renderable* item) :
                transform(transform),
                item(item)
            {}
        };

        using List = std::vector<Item>;

        void submit(const Matrix& transform, const Renderable& item)
        {
            m_items.emplace_back(transform, &item);
        }

		List::const_iterator begin() const { return m_items.begin(); }
		List::const_iterator end() const { return m_items.end(); }

        void clear()
        {
            m_items.clear();
        }
        
    private:

        List m_items;
    };
}
