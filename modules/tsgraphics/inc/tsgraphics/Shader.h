/*
	Shader object
*/

#pragma once

#include <tscore/types.h>
#include <tscore/path.h>

#include "Driver.h"

namespace ts
{
	class ShaderProgram
	{
	public:

		ShaderProgram() {}
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram(ShaderProgram&& rhs) : m_program(std::move(rhs.m_program)) {}
		
		//Construct a shader from the given file
		explicit ShaderProgram(RenderDevice* device, const String& shaderFile) { load(device, shaderFile); }

		//Load a shader from the given file
		bool load(RenderDevice* device, const String& shaderFile);

		//Shader device handle
		ShaderHandle handle() const { return m_program.handle(); }

		//Shader state
		bool loaded() const { return !m_program.null(); }
		operator bool() const { return loaded(); }

	private:

		RPtr<ShaderHandle> m_program;
	};
}