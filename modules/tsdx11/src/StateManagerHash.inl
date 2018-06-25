/*
	Render API

	D3D11StateManager hashes
*/

#pragma once

#include "Helpers.h"

#include <tuple>
#include <unordered_map>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	State entry hash/comparison implementations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct hash<DepthState>
	{
		size_t operator()(const DepthState& state) const
		{
			return (size_t)state.enableDepth << 1 ^ (size_t)state.enableStencil;
		}
	};

	template<>
	struct hash<RasterizerState>
	{
		size_t operator()(const RasterizerState& state) const
		{
			return (size_t)state.cullMode << 4 ^ (size_t)state.fillMode << 2 ^ (size_t)state.enableScissor;
		}
	};

	template<>
	struct hash<BlendState>
	{
		size_t operator()(const BlendState& state) const
		{
			return (size_t)state.enable;
		}
	};

	template<>
	struct hash<SamplerState>
	{
		size_t operator()(const SamplerState& state) const
		{
			return ((uint64)state.borderColour.get() << 32) ^ (size_t)state.addressU << 24 ^ (size_t)state.addressV << 16 ^ (size_t)state.addressW << 8 ^ (size_t)state.filtering;
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct equal_to<DepthState>
	{
		bool operator()(const DepthState& left, const DepthState& right) const
		{
			return tie(left.enableDepth, left.enableStencil) == tie(right.enableDepth, right.enableStencil);
		}
	};

	template<>
	struct equal_to<RasterizerState>
	{
		bool operator()(const RasterizerState& left, const RasterizerState& right) const
		{
			return tie(
				left.enableScissor,
				left.cullMode,
				left.fillMode
			) == tie(
				right.enableScissor,
				right.cullMode,
				right.fillMode
			);
		}
	};
	
	template<>
	struct equal_to<BlendState>
	{
		bool operator()(const BlendState& left, const BlendState& right) const
		{
			return (left.enable == right.enable);
		}
	};

	template<>
	struct equal_to<SamplerState>
	{
		bool operator()(const SamplerState& left, const SamplerState& right) const
		{
			return tie(
				left.addressU,
				left.addressV,
				left.addressW,
				left.borderColour,
				left.filtering
			) == tie(
				right.addressU,
				right.addressV,
				right.addressW,
				right.borderColour,
				right.filtering
			);
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
