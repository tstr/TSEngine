/*
	Shader compiler tool

	usage:
		
		shaderc -t manifest.tsc -o shaders/bin/

		-t : target manifest, specifies list of shaders to compile and how to compile them
		-s : location of shader source files
		-o : output directory for compiled shaders


	directory structure:
		
		shaders/
			bin/
				cache/
					00001/
						28f721f7391ae39.cache
						...

					00002/
						28f721f7391ae39.cache
						...

				shader.tsh
				...
					
			src/
				shader.vs
				shader.ps
				...

			manifest.tshc


	manifest.tshc example:
		
		shader Shader
		{
			stage vertex
			{
				file = "shader.vs";
			}

			stage pixel
			{
				file = "shader.ps";
			}
		}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "compiler.h"
#include "frontend/importer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace ts;

int main(int argc, char** argv)
{
	CShaderCompileEngine compileEngine(
		"C:/Users/Tom/Documents/Tom/git/TSEngine/tools/shaderc",
		"C:/Users/Tom/Documents/Tom/git/TSEngine/tools/shaderc/bin"
	);

	SShaderInfo inf;
	inf.vertexStage.sourceFile = "shader.vs";
	inf.vertexStage.entryPoint = "VS";
	inf.vertexStage.macroCount = 0;
	inf.vertexStage.macros = nullptr;
	inf.pixelStage.sourceFile = "shader.ps";
	inf.pixelStage.entryPoint = "PS";
	inf.pixelStage.macroCount = 0;
	inf.pixelStage.macros = nullptr;

	compileEngine.compileShader("shader", inf);

	return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
