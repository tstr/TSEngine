/*
	Render API
	
	D3D11StateManager:

	creates and caches states for reuse in multiple draw commands
*/

#pragma once

#include "base.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class D3D11StateManager
	{
	private:

		template<typename desc_t, typename state_t>
		struct StateEntry;
		
		template<typename desc_t, typename state_t>
		struct StateEntry
		{
			desc_t desc;
			ComPtr<state_t> state;

			StateEntry()
			{
				desc = desc_t();
				state.Reset();
			}

			StateEntry(desc_t desc, state_t* state = nullptr)
			{
				this->desc = desc;
				this->state = state;
			}

			bool operator==(const StateEntry& rhs);
		};

		template<typename desc_t, typename state_t>
		class StateCache
		{
		private:

			typedef StateEntry<desc_t, state_t> Entry_t;

			std::vector<Entry_t> m_cache;

		public:

			bool find(const desc_t& desc, state_t** state)
			{
				auto it = std::find(m_cache.begin(), m_cache.end(), desc);

				if (it != m_cache.end())
				{
					*state = it->state.Get();
					(*state)->AddRef();
					return true;
				}

				return false;
			}

			void insert(const desc_t& desc, state_t* state)
			{
				ComPtr<state_t> i;
				if (!this->find(desc, i.GetAddressOf()))
				{
					m_cache.push_back(Entry_t(desc, state));
				}
			}
		};

		ComPtr<ID3D11Device> m_device;

		//Caches
		StateCache<SDepthState, ID3D11DepthStencilState> m_cacheDepthState;
		StateCache<SRasterState, ID3D11RasterizerState> m_cacheRasterState;
		StateCache<SBlendState, ID3D11BlendState> m_cacheBlendState;
		StateCache<STextureSampler, ID3D11SamplerState> m_cacheSamplerState;

	public:

		D3D11StateManager() {}

		D3D11StateManager(ID3D11Device* device) :
			m_device(device)
		{}
		
		//State creation methods - finds states with the closest matching parameters
		HRESULT demandDepthState(const SDepthState& desc, ID3D11DepthStencilState** state);
		HRESULT demandRasterizerState(const SRasterState& desc, ID3D11RasterizerState** state);
		HRESULT demandBlendState(const SBlendState& desc, ID3D11BlendState** state);
		HRESULT demandSamplerState(const STextureSampler& desc, ID3D11SamplerState** state);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
