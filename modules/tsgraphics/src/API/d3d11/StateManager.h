/*
	Render API
	
	D3D11StateManager:

	creates and caches states for reuse in multiple draw commands
*/

#pragma once

#include "base.h"
#include <unordered_map>

#include "StateManagerHash.inl"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace std
{
	template<> struct hash<DepthState>;
	template<> struct hash<RasterizerState>;
	template<> struct hash<BlendState>;
	template<> struct hash<SamplerState>;
}

namespace ts
{
	class D3D11StateManager
	{
	private:

		template<typename desc_t, typename state_t>
		class StateCache
		{
		private:

			std::unordered_map<desc_t, ComPtr<state_t>> m_cache;

		public:

			bool find(const desc_t& desc, state_t** state)
			{
				auto it = m_cache.find(desc);

				if (it != m_cache.end())
				{
					*state = it->second.Get();
					(*state)->AddRef();
					return true;
				}

				return false;
			}

			void insert(const desc_t& desc, state_t* state)
			{
				m_cache.insert(make_pair(desc, ComPtr<state_t>(state)));
			}
		};

		ComPtr<ID3D11Device> m_device;

		//Caches
		StateCache<DepthState, ID3D11DepthStencilState> m_cacheDepthState;
		StateCache<RasterizerState, ID3D11RasterizerState> m_cacheRasterState;
		StateCache<BlendState, ID3D11BlendState> m_cacheBlendState;
		StateCache<SamplerState, ID3D11SamplerState> m_cacheSamplerState;

	public:

		D3D11StateManager() {}

		D3D11StateManager(ID3D11Device* device) :
			m_device(device)
		{}

		ID3D11Device* getDevice() const { return m_device.Get(); }
		
		//State creation methods - finds states with the closest matching parameters
		HRESULT demandDepthState(const DepthState& desc, ID3D11DepthStencilState** state);
		HRESULT demandRasterizerState(const RasterizerState& desc, ID3D11RasterizerState** state);
		HRESULT demandBlendState(const BlendState& desc, ID3D11BlendState** state);
		HRESULT demandSamplerState(const SamplerState& desc, ID3D11SamplerState** state);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
