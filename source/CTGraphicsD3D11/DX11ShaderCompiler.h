/*
	Effect compiler header
*/

#pragma once

#include "DX11Base.h"
#include "DX11Utils.h"

#include <CT\core\corecommon.h>

namespace CT
{
	namespace DX11
	{
		class ShaderModel
		{
		private:

			char stage[2];
			char r0 = '_';
			char major = 'x';
			char r1 = '_';
			char minor = 'x';
			char null = 0; //null terminating character

		public:

			inline void SetStage(char a[2]) { memcpy(stage, a, sizeof(char[2])); }

			inline void SetTargetMajor(char c) { major = c; }
			inline void SetTargetMinor(char c) { minor = c; }

			inline const char* tostring()
			{
				r0 = '_';
				r1 = '_';
				return reinterpret_cast<const char*>(this);
			}
		};

		struct ShaderCacheHeader
		{
			char entrypoint[32];
			ShaderModel model; //Shader model: vs_5_0, ps_5_0 etc.
			ABI::ShaderType type;
			uint32 size = 0;
		};

		class ShaderCompiler
		{

		};
	}
}