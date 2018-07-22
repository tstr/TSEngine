/*
    Forward rendering pipeline
*/

#pragma once

#include <vector>
#include <unordered_map>

#include <tscore/path.h>
#include <tsgraphics/Graphics.h>
#include <tsgraphics/Buffer.h>
#include <tsgraphics/Shader.h>
#include <tsgraphics/Model.h>
#include <tsgraphics/RenderTarget.h>

#include "Renderable.h"
#include "Material.h"
#include "ShadowMap.h"

#include "ForwardRenderConstants.h"

namespace ts
{
	using ForwardRenderTarget = RenderTargets<1>;

    class ForwardRenderer
    {
    public:

		ForwardRenderer() {}
		ForwardRenderer(const ForwardRenderer&) = delete;

		ForwardRenderer(GraphicsSystem* gfx);

		/*
			Set global constants
		*/

		void setCameraView(const Matrix& view) { m_viewMatrix = view; }
		void setCameraProjection(const Matrix& proj) { m_projMatrix = proj; }
		void setCamera(const Matrix& view, const Matrix& proj) { setCameraView(view); setCameraProjection(proj); }
		void setAmbientColour(const Vector& ambient) { m_ambientColour = ambient; }

		void setDirectionalLightColour(RGBA colour) { m_directLightColour = colour; }
		void setDirectionalLightDir(const Vector& dir) { m_directLightDir = dir; }

		using LightID = uint32;

		void enableDynamicLight(LightID light) { m_dynamicLights[light].enabled = true; }
		void disableDynamicLight(LightID light) { m_dynamicLights[light].enabled = false; }

		void setLightColour(LightID light, RGBA colour) { m_dynamicLights[light].colour = Vector(colour); }
		void setLightPosition(LightID light, const Vector& pos) { m_dynamicLights[light].pos = pos; }

		void setLightAttenuation(LightID light, float quadratic, float linear, float constant)
		{
			m_dynamicLights[light].attConstant = constant;
			m_dynamicLights[light].attLinear = linear;
			m_dynamicLights[light].attQuadratic = quadratic;
		}

		///////////////////////////////////////////////////////////////////////////////

        //Create renderable item
        Renderable createRenderable(const Mesh& mesh, const PhongMaterial&);

        //Draw a renderable item
        void draw(const Renderable& item, const Matrix& transform);

		void begin(ForwardRenderTarget& target);
		void end();

    private:

		///////////////////////////////////////////////////////////////////////////////

		GraphicsSystem* m_gfx = nullptr;

		TargetHandle m_targets;

		Buffer m_perMesh;
		Buffer m_perScene;

		ShaderProgram m_shader;
		ShaderProgram m_shaderNormMap;

		ShadowMap m_shadowMap;

		///////////////////////////////////////////////////////////////////////////////
		//	Properties
		///////////////////////////////////////////////////////////////////////////////

		Vector m_ambientColour;
		Matrix m_viewMatrix;
		Matrix m_projMatrix;

		RGBA m_directLightColour;
		Vector m_directLightDir;

		DynamicLight m_dynamicLights[MAX_LIGHTS];

		///////////////////////////////////////////////////////////////////////////////

		struct QueueElement
		{
			const Renderable* item = nullptr;
			Matrix transform;

			QueueElement() = default;
			QueueElement(const Renderable* i, const Matrix& t) :
				item(i),
				transform(t)
			{}
		};

		std::vector<QueueElement> m_renderQueue;

		///////////////////////////////////////////////////////////////////////////////

		void preloadShaders();
		
		ShaderHandle selectShader(const Mesh& mesh, const PhongMaterial& mat);
        RPtr<PipelineHandle> makePipeline(const Mesh& mesh, const PhongMaterial& mat);
		RPtr<ResourceSetHandle> makeResourceSet(const Mesh& mesh, const MaterialInstance& mat);

		void makeShadowPipelineAndResources(Renderable& item, const Mesh& mesh);
    };
}
