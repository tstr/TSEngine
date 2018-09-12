/*
    Material manager
*/

#pragma once

#include <tsgraphics/Graphics.h>
#include <tsgraphics/Shader.h>
#include "Material.h"

namespace ts
{
    class MaterialManager
    {
		GraphicsSystem* m_gfx;

		ShaderProgram m_shader, m_shaderNormMap;
		ShaderProgram m_shadowMapper;

    public:

		MaterialManager() {}
		MaterialManager(GraphicsSystem* gfx);
		MaterialManager(const MaterialManager&) = delete;
		MaterialManager(MaterialManager&& rhs) = default;

        RPtr<PipelineHandle> getForwardPipeline(const Mesh& mesh, const PhongMaterial& material);

        RPtr<PipelineHandle> getShadowPipeline(const Mesh& mesh, const PhongMaterial& material);

    private:

		void preloadShaders();

		ShaderHandle selectShader(const Mesh& mesh, const PhongMaterial& mat);

		void findAttribute(
			const char* semantic,
			VertexAttributeType type,
			const VertexAttributeMap& attribMap,
			std::vector<VertexAttribute>& attribs
		);
    };
}
