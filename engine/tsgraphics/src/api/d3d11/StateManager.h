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
	template<> struct hash<SDepthState>;
	template<> struct hash<SRasterState>;
	template<> struct hash<SBlendState>;
	template<> struct hash<STextureSampler>;

	/*
	template<>
	struct hash<SDepthState>
	{
		size_t operator()(const SDepthState& state) const;
	};

	template<>
	struct hash<SRasterState>
	{
		size_t operator()(const SRasterState& state) const;
	};

	template<>
	struct hash<SBlendState>
	{

		size_t operator()(const SBlendState& state) const;
	};

	template<>
	struct hash<STextureSampler>
	{
		size_t operator()(const STextureSampler& state) const;
	};

	template<>
	struct equal_to<SDepthState>
	{
		bool operator()(const SDepthState& left, const SDepthState& right) const;
	};

	template<>
	struct equal_to<SRasterState>
	{
		bool operator()(const SRasterState& left, const SRasterState& right) const;
	};

	template<>
	struct equal_to<SBlendState>
	{

		bool operator()(const SBlendState& left, const SBlendState& right) const;
	};


	template<>
	struct equal_to<STextureSampler>
	{
		bool operator()(const STextureSampler& left, const STextureSampler& right) const;
	};
	//*/
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
