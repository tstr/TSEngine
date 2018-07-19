/*
    Material reader
*/

#pragma once

#include <tscore/Path.h>
#include "ForwardRender.h"

namespace ts
{
    class MaterialReader
    {
    public:
        
		using MaterialMap = std::unordered_map<String, PhongMaterial>;
        
        MaterialReader(GraphicsSystem* gfx, const Path& fileName);
        
		PhongMaterial find(const String& name) const;
        bool has(const String& name) const;
        
        MaterialMap::const_iterator begin() const { return m_infoMap.begin(); }
        MaterialMap::const_iterator end() const { return m_infoMap.end(); }
        
    private:

        MaterialMap m_infoMap;
    };
}