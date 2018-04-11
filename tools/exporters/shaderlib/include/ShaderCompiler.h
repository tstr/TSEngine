/*
	Shader Compiler Class
*/

#pragma once

#include "ShaderCommon.h"

#include <tscore/path.h>
#include <tscore/system/memory.h>
#include <tscore/ptr.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	/*
		Options for the shader compiler
	*/
	struct ShaderCompilerOptions
	{

		//Entry points for each stage
		String vsEntry;  //vertex stage
		String psEntry;  //pixel stage
		String gsEntry;	 //geometry stage
		String tcsEntry; //tessellator control stage
		String tesEntry; //tessellator evaluation stage
	};

	class ShaderCompiler
	{
	private:

		struct Impl;
		OpaquePtr<Impl> pCompiler;

	public:

		ShaderCompiler();
		~ShaderCompiler();

		OPAQUE_PTR(ShaderCompiler, pCompiler)
			
		int compile(std::istream& source, std::ostream& output, const ShaderCompilerOptions& options);
	};
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
