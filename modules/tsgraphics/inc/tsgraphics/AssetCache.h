/*
    Asset cache helper
*/

#pragma once

#include <unordered_map>

#include <tscore/path.h>

namespace ts
{
    /*
        Generic asset caches
    */
    template<class Derived, class AssetType>
    class AssetCache
    {
        using InternalCache = std::unordered_map<Path, AssetType>;

    public:

        AssetType& get(const Path& filePath)
        {
			using std::make_pair;

            auto it = m_cache.find(filePath);

            if (it == m_cache.end())
            {
				Derived* d = static_cast<Derived*>(this);
				return m_cache.emplace(filePath, d->load(filePath)).first->second;
				//return m_cache.find(filePath)->second;
            }

			return it->second;
        }

    private:

        InternalCache m_cache;
    };
}
