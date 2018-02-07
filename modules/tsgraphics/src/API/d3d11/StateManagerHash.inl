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
	struct hash<SDepthState>
	{
		size_t operator()(const SDepthState& state) const
		{
			return (size_t)state.enableDepth << 1 ^ (size_t)state.enableStencil;
		}
	};

	template<>
	struct hash<SRasterState>
	{
		size_t operator()(const SRasterState& state) const
		{
			return (size_t)state.cullMode << 4 ^ (size_t)state.fillMode << 2 ^ (size_t)state.enableScissor;
		}
	};

	template<>
	struct hash<SBlendState>
	{
		size_t operator()(const SBlendState& state) const
		{
			return (size_t)state.enable;
		}
	};

	template<>
	struct hash<STextureSampler>
	{
		size_t operator()(const STextureSampler& state) const
		{
			return ((uint64)state.borderColour.get() << 32) ^ (size_t)state.addressU << 24 ^ (size_t)state.addressV << 16 ^ (size_t)state.addressW << 8 ^ (size_t)state.filtering;
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct equal_to<SDepthState>
	{
		bool operator()(const SDepthState& left, const SDepthState& right) const
		{
			return tie(left.enableDepth, left.enableStencil) == tie(right.enableDepth, right.enableStencil);
		}
	};

	template<>
	struct equal_to<SRasterState>
	{
		bool operator()(const SRasterState& left, const SRasterState& right) const
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
	struct equal_to<SBlendState>
	{
		bool operator()(const SBlendState& left, const SBlendState& right) const
		{
			return (left.enable == right.enable);
		}
	};

	template<>
	struct equal_to<STextureSampler>
	{
		bool operator()(const STextureSampler& left, const STextureSampler& right) const
		{
			return tie(
				left.enabled,
				left.addressU,
				left.addressV,
				left.addressW,
				left.borderColour,
				left.filtering
			) == tie(
				right.enabled,
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
